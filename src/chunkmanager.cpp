#include "chunkmanager.h"

#include "gamewindow.h"

#include <iostream>

#include <QtGui/QOpenGLVertexArrayObject>
#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QOpenGLTexture>

ChunkManager::ChunkManager() : m_isInit{false}, m_chunkBuffers{nullptr},
                            m_oglBuffers{nullptr}, m_currentChunkI{-1},
							m_currentChunkJ{ -1 }, m_currentChunkK{ -1 }, m_ChunkGenerator(){
    m_chunkBuffers = new Voxel[CHUNK_NUMBER * CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE];
    memset(m_chunkBuffers, 1, CHUNK_NUMBER * CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE * sizeof(Voxel));

    m_needRegen = true;
    m_generationIsRunning = false;

    m_availableChunkData = new std::atomic<bool>[CHUNK_NUMBER];
    m_inUseChunkData = new std::atomic<bool>[CHUNK_NUMBER];
    for (int i = 0; i < CHUNK_NUMBER; ++i) {
        m_availableChunkData[i] = true;
        m_inUseChunkData[i] = false;
    }
    m_availableBuffer = new std::atomic<bool>[VBO_NUMBER];
    for (int i = 0; i < VBO_NUMBER; ++i) {
        m_availableBuffer[i] = true;
    }
    m_tempVertexData = new GLuint[CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE * 6 * 6];
    m_vboToUpload = 0;
    m_canGenerateMesh = true;
    m_canUploadMesh = false;
    m_countToUpload = 0;
}

ChunkManager::~ChunkManager() {
    QThread::terminate();
    wait();
    delete[] m_chunkBuffers;
    delete[] m_availableChunkData;
    delete[] m_inUseChunkData;
    delete[] m_availableBuffer;
    delete[] m_tempVertexData;
}

void ChunkManager::initialize(GameWindow* gl) {

    m_program = new QOpenGLShaderProgram(gl);
    m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, QString(":/render.vs"));
    m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, QString(":/render.ps"));
	if (!m_program->link()) {
		// TODO(antoine): Remove Force crash
		abort();
	}
    m_posAttr = m_program->attributeLocation("position");
    m_matrixUniform = m_program->uniformLocation("viewProj");
    m_chunkPosUniform = m_program->uniformLocation("chunkPosition");

    m_program->bind();
    m_program->setUniformValue("atlas", 0);
    m_program->setUniformValue("tileCount", 16);
    m_program->setUniformValue("tileSize", 16);
    m_program->release();

    m_atlas = new QOpenGLTexture(QImage(":/atlas.png"));
    m_atlas->setMagnificationFilter(QOpenGLTexture::Nearest);

    GLuint vbos[VBO_NUMBER];
    gl->glGenBuffers(VBO_NUMBER, vbos);

    m_oglBuffers = new Buffer[VBO_NUMBER];
    for (int i = 0; i < VBO_NUMBER; ++i) {
        Buffer* buffer = m_oglBuffers + i;
        buffer->count = 0;
        buffer->draw = true;
        buffer->vao = new QOpenGLVertexArrayObject(gl);
        buffer->vao->create();
        buffer->vao->bind();
        buffer->vbo = vbos[i];
        gl->glBindBuffer(GL_ARRAY_BUFFER, buffer->vbo);
        gl->glEnableVertexAttribArray(m_posAttr);
        gl->glVertexAttribIPointer(m_posAttr, 1, GL_UNSIGNED_INT, 0, 0);
        buffer->vao->release();
    }

    m_isInit = true;
}

void ChunkManager::destroy(GameWindow* gl) {
    m_isInit = false;

    // On nettoie les ressources opengl
    for (int i = 0; i < VBO_NUMBER; ++i) {
        Buffer* buffer = m_oglBuffers + i;
        delete buffer->vao;
        gl->glDeleteBuffers(1, &buffer->vbo);
    }

    // On supprime les tableaux
    delete[] m_oglBuffers;
    m_oglBuffers = nullptr;
    // On supprime l'atlas et le shader
    delete m_atlas;
    m_atlas = nullptr;
    delete m_program;
    m_program = nullptr;
}

void ChunkManager::update(GameWindow* gl) {
    if (m_isInit) {
        // On débloque l'accès aux données (en cas d'oublie d'un appel à unlockChunkData)
        for (int i = 0; i < CHUNK_NUMBER; ++i) {
            m_inUseChunkData[i].store(false);
        }

        QVector3D camPos = gl->getCamera().getPosition();
        int chunkI = floor(camPos.x() / (CHUNK_SIZE * CHUNK_SCALE));
        int chunkJ = floor(camPos.y() / (CHUNK_SIZE * CHUNK_SCALE));
        int chunkK = floor(camPos.z() / (CHUNK_SIZE * CHUNK_SCALE));

        // On nettoie les chunks inutiles
        if (!m_generationIsRunning) {
            for (auto it = m_ChunkMap.begin(); it != m_ChunkMap.end();) {
                if ((it->second.chunkBufferIndex == -1) && (it->second.vboIndex == -1)) {
                    it = m_ChunkMap.erase(it);
                } else {
                    ++it;
                }
            }
        }
        // Si on change de chunk on lance la génération
        if ((chunkI != m_currentChunkI) || (chunkK != m_currentChunkK)) {
            m_currentChunkI = chunkI;
            m_currentChunkJ = chunkJ;
            m_currentChunkK = chunkK;

            m_needRegen.store(true);

            start();
        }
        if (m_canUploadMesh) {
            m_canUploadMesh = false;
            m_oglBuffers[m_vboToUpload].count = m_countToUpload;
            gl->glBindBuffer(GL_ARRAY_BUFFER, m_oglBuffers[m_vboToUpload].vbo);
            gl->glBufferData(GL_ARRAY_BUFFER, m_countToUpload * sizeof(GLuint), m_tempVertexData, GL_DYNAMIC_DRAW);
            gl->glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
        m_canGenerateMesh = true;
    }
}

void ChunkManager::draw(GameWindow* gl) {
    if (m_isInit) {
        m_program->bind();

        QMatrix4x4 mat = gl->getCamera().getViewProjMatrix();
        QMatrix4x4 scale;
        scale.scale(CHUNK_SCALE, CHUNK_SCALE, CHUNK_SCALE);
        m_program->setUniformValue(m_matrixUniform, mat * scale);
        m_atlas->bind(0);

        for (auto& it : m_ChunkMap) {
            Chunk& chunk = it.second;
            if (chunk.vboIndex != -1) {
                Buffer* buffer = m_oglBuffers + chunk.vboIndex;
                if (buffer->draw) {
                    buffer->vao->bind();
                    m_program->setUniformValue(m_chunkPosUniform, QVector3D(chunk.i * CHUNK_SIZE, chunk.j * CHUNK_SIZE, chunk.k * CHUNK_SIZE));
                    gl->glDrawArrays(GL_TRIANGLES, 0, buffer->count);
                    buffer->vao->release();
                }
            }
        }
    }
}

Voxel* ChunkManager::lockChunkData(int i, int j, int k) {
    Chunk& chunk = getChunk(i, j, k);
    int index = chunk.chunkBufferIndex;
    if (index >= 0) {
        // On bloque le buffer
        m_inUseChunkData[i].store(true);

        // Si le buffer est disponible on le renvoie
        if (m_availableBuffer[i].load()) {
            return m_chunkBuffers + (index * CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE);
        }
        // Le buffer n'est pas disponible => on le débloque
        m_inUseChunkData[i].store(false);
    }
    return nullptr;
}

void ChunkManager::unlockChunkData(int i, int j, int k) {
    Chunk& chunk = getChunk(i, j, k);
    int index = chunk.chunkBufferIndex;
    if (index >= 0) {
        //On débloque le buffer
        m_inUseChunkData[i].store(false);
    }
}

Chunk& ChunkManager::getChunk(int i, int j, int k) {
    auto tuple = std::make_tuple(i, j, k);
    auto it = m_ChunkMap.find(tuple);
    if (it == m_ChunkMap.end()) {
        Chunk chunk;
        chunk.i = i;
        chunk.j = j;
        chunk.k = k;
        chunk.chunkBufferIndex = -1;
        chunk.vboIndex = -1;
        chunk.visible = true;
        m_ChunkMap[tuple] = chunk;
    }
    return m_ChunkMap[tuple];
}

void ChunkManager::run() {
    while (m_needRegen) {
        m_needRegen.store(false);
        m_generationIsRunning.store(true);

        m_toInvalidateChunkData.clear();
        m_toInvalidateBuffer.clear();
        m_toGenerateChunkData.clear();
        m_toGenerateBuffer.clear();

        // On invalide les chunks en dehors de la vue
        for (auto& it : m_ChunkMap) {
            if (m_needRegen) {
                break;
            }
            Chunk& chunk = it.second;
            if ((std::abs(chunk.i - m_currentChunkI) > VIEW_SIZE) ||
                (std::abs(chunk.k - m_currentChunkK) > VIEW_SIZE)) {
                if (chunk.chunkBufferIndex != -1) {
                    m_availableChunkData[chunk.chunkBufferIndex].store(false);
                    m_toInvalidateChunkData.push_back(it.first);
                }
                if (chunk.vboIndex != -1) {
                    m_availableBuffer[chunk.vboIndex].store(false);
                    m_toInvalidateBuffer.push_back(it.first);
                }
            }
        }
        if (m_needRegen) {
            break;
        }
        // On recherche tous les chunks nécessaires
        for (int i = m_currentChunkI - VIEW_SIZE; i <= m_currentChunkI + VIEW_SIZE; ++i) {
            for (int k = m_currentChunkK - VIEW_SIZE; k <= m_currentChunkK + VIEW_SIZE; ++k) {
                for (int j = 0; j < 7; ++j) {
                    if (m_needRegen) {
                        break;
                    }
                    auto tuple = std::make_tuple(i, j, k);
                    auto it = m_ChunkMap.find(tuple);
                    if (it != m_ChunkMap.end()) {
                        Chunk& chunk = it->second;
                        chunk.i = i;
                        chunk.j = j;
                        chunk.k = k;
                        if (chunk.chunkBufferIndex == -1) {
                            m_toGenerateChunkData.push_back(tuple);
                        }
                        if (chunk.vboIndex == -1) {
                            m_toGenerateBuffer.push_back(tuple);
                        }
                    } else {
                        Chunk chunk;
                        chunk.i = i;
                        chunk.j = j;
                        chunk.k = k;
                        chunk.chunkBufferIndex = -1;
                        chunk.vboIndex = -1;
                        chunk.visible = true;
                        m_toGenerateChunkData.push_back(tuple);
                        m_toGenerateBuffer.push_back(tuple);
                        m_ChunkMap[tuple] = chunk;
                    }
                }
            }
        }
        if (m_needRegen) {
            break;
        }
        // On génére tous les chunk
        while (m_toGenerateChunkData.size() > 0) {
            if (m_needRegen) {
                break;
            }
            // Le chunk à créer
            std::tuple<int, int, int> newChunkPos = m_toGenerateChunkData.back();
            m_toGenerateChunkData.pop_back();
            Chunk& newChunk = getChunk(std::get<0>(newChunkPos), std::get<1>(newChunkPos), std::get<2>(newChunkPos));

            int bufferIndex = -1;
            if (m_toInvalidateChunkData.size() > 0) {
                // Le chunk à supprimer
                std::tuple<int, int, int> oldChunkPos = m_toInvalidateChunkData.back();
                m_toInvalidateChunkData.pop_back();
                Chunk& oldChunk = getChunk(std::get<0>(oldChunkPos), std::get<1>(oldChunkPos), std::get<2>(oldChunkPos));
                bufferIndex = oldChunk.chunkBufferIndex;
                // TODO sauvegarder l'ancien chunk
                oldChunk.chunkBufferIndex = -1;
            } else {
                bufferIndex = seekFreeChunkData();
            }
            if (bufferIndex != -1) {
                newChunk.chunkBufferIndex = bufferIndex;
                m_availableChunkData[bufferIndex].store(false);
				
                // TODO Générer le nouveau chunk
				
				Voxel* data = m_chunkBuffers + (newChunk.chunkBufferIndex * CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE);
				m_ChunkGenerator.generateChunk(data, newChunk.i, newChunk.j, newChunk.k);
				
            } else {
                // Plus assez de blocs libres
                #ifdef QT_DEBUG
                std::cout << "Plus assez de blocs libre pour charger un chunk" << std::endl;
                #endif
                break;
            }
        }
        if (m_needRegen) {
            break;
        }
        // On transforme tous les chunks en meshs.
        while (m_toGenerateBuffer.size() > 0) {
            while (m_canGenerateMesh == false) {
                if (m_needRegen) {
                    break;
                }
            }
            m_canGenerateMesh = false;
            if (m_needRegen) {
                break;
            }
            // Le chunk à créer
            std::tuple<int, int, int> newChunkPos = m_toGenerateBuffer.back();
            m_toGenerateBuffer.pop_back();
            Chunk& newChunk = getChunk(std::get<0>(newChunkPos), std::get<1>(newChunkPos), std::get<2>(newChunkPos));
            if (newChunk.chunkBufferIndex !=-1) {
                Voxel* data = m_chunkBuffers + (newChunk.chunkBufferIndex * CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE);

                if (m_toInvalidateBuffer.size() > 0) {
                    // Le chunk à supprimer
                    std::tuple<int, int, int> oldChunkPos = m_toInvalidateBuffer.back();
                    m_toInvalidateBuffer.pop_back();
                    Chunk& oldChunk = getChunk(std::get<0>(oldChunkPos), std::get<1>(oldChunkPos), std::get<2>(oldChunkPos));
                    m_vboToUpload = oldChunk.vboIndex;
                    oldChunk.vboIndex = -1;
                } else {
                    m_vboToUpload = seekFreeBuffer();
                }
                if (m_vboToUpload != -1) {
                    newChunk.vboIndex = m_vboToUpload;

                    m_countToUpload = m_meshGenerator.generate(data, m_tempVertexData);

                    m_availableBuffer[m_vboToUpload].store(false);
                    m_canUploadMesh = true;
                } else {
                    // Plus assez de vbo libres
                    #ifdef QT_DEBUG
                    std::cout << "Plus assez de vbo libre pour charger un chunk" << std::endl;
                    #endif
                    break;
                }
            } else {
                #ifdef QT_DEBUG
                std::cout << "Impossible de generer un mesh à partir d'un chunk non charge" << std::endl;
                #endif
            }
        }
        m_generationIsRunning.store(false);
    }
}

int ChunkManager::seekFreeChunkData() {
    for (int i = 0; i < CHUNK_NUMBER; ++i) {
        if (m_availableChunkData[i] == true) {
            return i;
        }
    }
    return -1;
}

int ChunkManager::seekFreeBuffer() {
    for (int i = 0; i < VBO_NUMBER; ++i) {
        if (m_availableBuffer[i] == true) {
            return i;
        }
    }
    return -1;
}
