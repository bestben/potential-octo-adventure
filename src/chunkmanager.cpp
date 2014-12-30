#include "chunkmanager.h"

#include "gamewindow.h"

#include <iostream>

#include <QtGui/QOpenGLVertexArrayObject>
#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QOpenGLTexture>

ChunkManager::ChunkManager() : m_isInit{false}, m_chunkBuffers{nullptr},
                            m_oglBuffers{ nullptr }, m_ChunkGenerator(), m_FirstUpdate{true}{
    m_chunkBuffers = new Voxel[CHUNK_NUMBER * CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE];
    memset(m_chunkBuffers, 0, CHUNK_NUMBER * CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE * sizeof(Voxel));

    m_chunkToDrawCount = 0;
    m_chunkToDraw = new Chunk*[VBO_NUMBER];
    for (int i = 0; i < VBO_NUMBER; ++i) {
        m_chunkToDraw[i] = nullptr;
    }

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

	m_currentChunk = { 0, -1, 0 };

}

ChunkManager::~ChunkManager() {
    QThread::terminate();
    wait();
	for (auto* c : m_ChunkMap)
		delete c;
    delete[] m_chunkBuffers;
    delete[] m_availableChunkData;
    delete[] m_inUseChunkData;
    delete[] m_availableBuffer;
    delete[] m_tempVertexData;
    delete[] m_chunkToDraw;
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
	m_program->setUniformValue("fogDistance", (float)(VIEW_SIZE*CHUNK_SIZE*CHUNK_SCALE));
	// TODO: Utiliser la couleur du ciel à la place
	m_program->setUniformValue("fogColor", QVector4D(.5f,.5f,.5f,1.0f));
    m_program->release();

    m_waterProgram = new QOpenGLShaderProgram(gl);
    m_waterProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, QString(":/water.vs"));
    m_waterProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, QString(":/water.ps"));
    if (!m_waterProgram->link()) {
        // TODO(antoine): Remove Force crash
        abort();
    }
    m_waterMatrixUniform = m_waterProgram->uniformLocation("viewProj");
    m_waterChunkPosUniform = m_waterProgram->uniformLocation("chunkPosition");
    m_waterProgram->bind();
    m_waterProgram->setUniformValue("atlas", 0);
    m_waterProgram->setUniformValue("tileCount", 16);
    m_waterProgram->setUniformValue("tileSize", 16);
    m_waterProgram->setUniformValue("fogDistance", (float)(VIEW_SIZE*CHUNK_SIZE*CHUNK_SCALE));
    // TODO: Utiliser la couleur du ciel à la place
    m_waterProgram->setUniformValue("fogColor", QVector4D(.5f,.5f,.5f,1.0f));
    m_waterProgram->release();

    m_atlas = new QOpenGLTexture(QImage(":/atlas.png"));
    m_atlas->setMagnificationFilter(QOpenGLTexture::Nearest);

    GLuint vbos[VBO_NUMBER];
    gl->glGenBuffers(VBO_NUMBER, vbos);

    m_oglBuffers = new Buffer[VBO_NUMBER];
    for (int i = 0; i < VBO_NUMBER; ++i) {
        Buffer* buffer = m_oglBuffers + i;
        buffer->opaqueCount = 0;
        buffer->waterCount = 0;
        buffer->draw = false;
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

	start();
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
    delete m_waterProgram;
    m_waterProgram = nullptr;
}

void ChunkManager::update(GameWindow* gl) {
    m_chunkToDrawCount = 0;
    if (m_isInit) {

        // On débloque l'accès aux données (en cas d'oublie d'un appel à unlockChunkData)
        /*for (int i = 0; i < CHUNK_NUMBER; ++i) {
            m_inUseChunkData[i].store(false);
        }*/

		//TODO: Fix le thread qui plante :(
		if (!isRunning()) {
			m_mutexChunkManagerList.unlock();
			start();
		}
			

        QVector3D camPos = gl->getCamera().getPosition();

		Coords chunkHere = voxelGetChunk(worldToVoxel(camPos));

        // On nettoie les chunks inutiles
		m_mutexChunkManagerList.lock();
		for (auto ite = m_ChunkMap.begin(); ite != m_ChunkMap.end();) {
			
			auto it = *ite;
			if ((std::abs(it->i - m_currentChunk.i) > VIEW_SIZE) ||
				(std::abs(it->k - m_currentChunk.k) > VIEW_SIZE)) {
				it->visible = false;

				if (it->chunkBufferIndex != -1) {
					m_availableChunkData[it->chunkBufferIndex] = true;
					it->chunkBufferIndex = -1;
				}
				if (it->vboIndex != -1) {
					m_oglBuffers[it->vboIndex].draw = false;
					m_availableBuffer[it->vboIndex] = true;
					it->vboIndex = -1;
				}
				ite = m_ChunkMap.erase(ite);
			} else {
				++ite;
			}
			// TODO: Save le chunk sur le DD pour réutilisation
			//++ite;
        }
		m_mutexChunkManagerList.unlock();
        
        // Si on change de chunk on demande des nouveaux chunks
		if ((chunkHere.i != m_currentChunk.i) || (chunkHere.k != m_currentChunk.k) || m_FirstUpdate) {

			m_currentChunk = chunkHere;

			requestChunks();
        }
		m_mutexChunkManagerList.lock();
        if (m_canUploadMesh) {
            m_canUploadMesh = false;
            gl->glBindBuffer(GL_ARRAY_BUFFER, m_oglBuffers[m_vboToUpload].vbo);
            gl->glBufferData(GL_ARRAY_BUFFER, m_countToUpload * sizeof(GLuint), m_tempVertexData, GL_DYNAMIC_DRAW);
            gl->glBindBuffer(GL_ARRAY_BUFFER, 0);
            m_oglBuffers[m_vboToUpload].opaqueCount = m_tempBufferToUpload.opaqueCount;
            m_oglBuffers[m_vboToUpload].waterCount = m_tempBufferToUpload.waterCount;

			m_oglBuffers[m_vboToUpload].draw = true;
			m_vboToUpload = -1;
        }
        m_canGenerateMesh = true;

        float camX = gl->getCamera().getPosition().x();
        float camY = gl->getCamera().getPosition().y();
        float camZ = gl->getCamera().getPosition().z();
        for (auto* chunk : m_ChunkMap) {
            if (chunk->vboIndex != -1 && chunk->visible) {
                m_chunkToDraw[m_chunkToDrawCount++] = chunk;
                int dx = camX - chunk->i;
                int dy = camY - chunk->j;
                int dz = camZ - chunk->k;
                chunk->distanceFromCamera = dx * dx + dy * dy + dz * dz;
            }
        }

        std::sort(m_chunkToDraw, m_chunkToDraw + m_chunkToDrawCount, [this](Chunk* i, Chunk* j)->bool{
            return i->distanceFromCamera < j->distanceFromCamera;
        });

		m_mutexChunkManagerList.unlock();
		
		m_FirstUpdate = false;
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
		m_mutexChunkManagerList.lock();

        for (int i = 0; i < m_chunkToDrawCount; ++i) {
            Chunk* chunk = m_chunkToDraw[i];
            Buffer* buffer = m_oglBuffers + chunk->vboIndex;

            if (buffer->draw && buffer->opaqueCount > 0) {
                buffer->vao->bind();
                m_program->setUniformValue(m_chunkPosUniform, QVector3D(chunk->i * CHUNK_SIZE, chunk->j * CHUNK_SIZE, chunk->k * CHUNK_SIZE));
                gl->glDrawArrays(GL_TRIANGLES, 0, buffer->opaqueCount);
                buffer->vao->release();
            }
        }
        //glDisable(GL_CULL_FACE);
        m_waterProgram->bind();
        m_waterProgram->setUniformValue(m_waterMatrixUniform, mat * scale);
        for (int i = m_chunkToDrawCount - 1; i >= 0; --i) {
            Chunk* chunk = m_chunkToDraw[i];
            Buffer* buffer = m_oglBuffers + chunk->vboIndex;
            if (buffer->draw && buffer->waterCount > 0) {
                buffer->vao->bind();
                m_waterProgram->setUniformValue(m_waterChunkPosUniform, QVector3D(chunk->i * CHUNK_SIZE, chunk->j * CHUNK_SIZE, chunk->k * CHUNK_SIZE));
                gl->glDrawArrays(GL_TRIANGLES, buffer->opaqueCount, buffer->waterCount);
                buffer->vao->release();
            }
        }
        //glEnable(GL_CULL_FACE);
		m_mutexChunkManagerList.unlock();

    }
}

void ChunkManager::checkChunk(Coords tuple) {
	auto it = m_ChunkMap.find(tuple);
	if (it != m_ChunkMap.end()) {
		Chunk* chunk = *it;
		if ((chunk->chunkBufferIndex == -1 || chunk->vboIndex == -1) && !m_toGenerateChunkData.contains(chunk) && !m_toGenerateBuffer.contains(chunk)) {
			m_toGenerateChunkData.push_back(chunk);
		} else {
			chunk->visible = true;
		}
	}
	else {
		// Chunk does not exist
        Chunk* chunk = new Chunk();
        chunk->i = tuple.i;
        chunk->j = tuple.j;
        chunk->k = tuple.k;
        chunk->chunkBufferIndex = -1;
        chunk->vboIndex = -1;
        chunk->visible = false;
        m_ChunkMap[tuple] = chunk;
        m_toGenerateChunkData.push_back(m_ChunkMap[tuple]);
    }
}

void ChunkManager::requestChunks() {

	m_mutexChunkManagerList.lock();
	int startJ = std::max(0, std::min(m_currentChunk.j, 6));
	for (int i = 0; i <= VIEW_SIZE; ++i) {
		for (int k = 0; k <= VIEW_SIZE; ++k) {
			for (int j = 0; j < 7; ++j) {

				if (startJ + j < 7){
					checkChunk({ m_currentChunk.i + i, startJ + j, m_currentChunk.k + k });
					checkChunk({ m_currentChunk.i - i, startJ + j, m_currentChunk.k + k });
					checkChunk({ m_currentChunk.i + i, startJ + j, m_currentChunk.k - k });
					checkChunk({ m_currentChunk.i - i, startJ + j, m_currentChunk.k - k });
				}
				if (startJ - j >= 0){
					checkChunk({ m_currentChunk.i + i, startJ - j, m_currentChunk.k + k });
					checkChunk({ m_currentChunk.i - i, startJ - j, m_currentChunk.k + k });
					checkChunk({ m_currentChunk.i + i, startJ - j, m_currentChunk.k - k });
					checkChunk({ m_currentChunk.i - i, startJ - j, m_currentChunk.k - k });
				}

			}
		}
	}
	m_mutexChunkManagerList.unlock();
}



Voxel* ChunkManager::getBufferAdress(int index) {
	return m_chunkBuffers + (index * CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE);
}


Chunk& ChunkManager::getChunk(Coords pos) {
	auto it = m_ChunkMap.find(pos);
	if (it == m_ChunkMap.end()) {
		Chunk *chunk = new Chunk();
		chunk->i = pos.i;
		chunk->j = pos.j;
		chunk->k = pos.k;
		chunk->chunkBufferIndex = -1;
		chunk->vboIndex = -1;
		chunk->visible = false;
		m_ChunkMap[pos] = chunk;
		return *m_ChunkMap[pos];
	}
	return **it;
}

Chunk& ChunkManager::getChunk(int i, int j, int k) {
	return getChunk({ i, j, k });
}

void ChunkManager::run() {
	
    while (m_needRegen) {
        // On génére tous les chunks requis
		m_mutexChunkManagerList.lock();
		
        if (m_toGenerateChunkData.size() > 0) {
            // Le chunk à créer
			

			auto* newChunk = m_toGenerateChunkData.front();
            m_toGenerateChunkData.pop_front();       

            int bufferIndex = seekFreeChunkData();
			int vboIndex = seekFreeBuffer();
	
			if (bufferIndex != -1 && vboIndex != -1) {
				newChunk->chunkBufferIndex = bufferIndex;
				newChunk->vboIndex = vboIndex;

				m_availableChunkData[bufferIndex] = false;
				m_availableBuffer[vboIndex] = false;
				m_mutexChunkManagerList.unlock();

                // TODO Générer le nouveau chunk et le prendre du DD si déja présent
				Voxel* data = getBufferAdress(bufferIndex);
				m_ChunkGenerator.generateChunk(data, newChunk->i, newChunk->j, newChunk->k);

				m_mutexChunkManagerList.lock();
				m_toGenerateBuffer.push_back(newChunk);
				
            } else {
                // Plus assez de blocs libres

                #ifdef QT_DEBUG
                std::cout << "Plus assez de blocs libre pour charger un chunk" << std::endl;
                #endif
            }
        }

        // On transforme tous les chunks en meshs.
		if (m_toGenerateBuffer.size() > 0 && m_canGenerateMesh) {
			m_canGenerateMesh = false;

            // Le chunk à créer
			auto* newChunk = m_toGenerateBuffer.front();
			m_toGenerateBuffer.pop_front();
			
			if (newChunk->chunkBufferIndex != -1) {
				if (newChunk->vboIndex != -1) {

					m_vboToUpload = newChunk->vboIndex;
					m_availableBuffer[m_vboToUpload] = false;
					m_oglBuffers[m_vboToUpload].draw = false;

					Voxel *data = getBufferAdress(newChunk->chunkBufferIndex);
                    m_countToUpload = m_meshGenerator.generate(data, &m_tempBufferToUpload, m_tempVertexData);

					m_canUploadMesh = true;
					newChunk->visible = true;
                } else {
                    // Plus assez de vbo libres
                    #ifdef QT_DEBUG
                    std::cout << "Plus assez de vbo libre pour charger un chunk" << std::endl;
                    #endif
                }
            } else {
                #ifdef QT_DEBUG
                std::cout << "Impossible de generer un mesh à partir d'un chunk non charge" << std::endl;
                #endif
            }
			
        }
		m_mutexChunkManagerList.unlock();
		// Eviter de faire fondre le cpu dans des boucles vides ;)


		QThread::msleep(5);
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

Voxel ChunkManager::getVoxel(int x, int y, int z) {
	Voxel res = {};

    Chunk& chunk = getChunk(div_floor(x, CHUNK_SIZE), div_floor(y, CHUNK_SIZE), div_floor(z, CHUNK_SIZE));
    if (chunk.chunkBufferIndex != -1) {
        Voxel* voxels = getBufferAdress(chunk.chunkBufferIndex);
        if (voxels != nullptr) {
			Coords c = voxelCoordsToChunkCoords({ x, y, z });
            res = voxels[c.i + CHUNK_SIZE * (c.j + CHUNK_SIZE * c.k)];
        }
    }
    return res;
}

VoxelType ChunkManager::setVoxel(int x, int y, int z, VoxelType newType) {
    VoxelType res = VoxelType::AIR;

    Chunk& chunk = getChunk(div_floor(x, CHUNK_SIZE), div_floor(y, CHUNK_SIZE), div_floor(z, CHUNK_SIZE));
    if (chunk.chunkBufferIndex != -1) {
        Voxel* voxels = getBufferAdress(chunk.chunkBufferIndex);
        if (voxels != nullptr) {
            Coords c = voxelCoordsToChunkCoords({ x, y, z });
            res = voxels[c.i + CHUNK_SIZE * (c.j + CHUNK_SIZE * c.k)].type;
            voxels[c.i + CHUNK_SIZE * (c.j + CHUNK_SIZE * c.k)].type = newType;
            // On lance la regen du mesh
            m_mutexChunkManagerList.lock();
            m_toGenerateBuffer.push_back(&chunk);
            m_mutexChunkManagerList.unlock();
        }
    }
    return res;
}
