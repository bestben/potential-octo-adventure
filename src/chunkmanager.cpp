#include "chunkmanager.h"
#include "gamewindow.h"
#include "save.h"

#include <iostream>

#include <QtGui/QOpenGLVertexArrayObject>
#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QOpenGLTexture>
#include <QtCore/QSet>

ChunkManager::ChunkManager(int worldSeed) : m_isInit{ false },
m_chunkBuffers{ nullptr }, m_oglBuffers{ nullptr }, m_FirstUpdate{ true }{
	mWorldSeed = worldSeed;
	m_LightManager = new LightManager(this);
	m_meshGenerator = new MeshGenerator(this);
	m_ChunkGenerator = new ChunkGenerator(this, worldSeed);
	m_chunkBuffers = new Voxel[CHUNK_NUMBER * CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE];
	memset(m_chunkBuffers, 0, CHUNK_NUMBER * CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE * sizeof(Voxel));

	m_chunkToDrawCount = 0;
	m_chunkToDraw = new Chunk*[VBO_NUMBER];
	for (int i = 0; i < VBO_NUMBER; ++i) {
		m_chunkToDraw[i] = nullptr;
	}

	m_needRegen = true;

	m_tempVertexData = new GLuint[CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE * 6 * 6];
	m_vboToUpload = 0;
	m_canGenerateMesh = true;
	m_canUploadMesh = false;
	m_countToUpload = 0;

	m_currentChunk = { 0, -1, 0 };
	m_animationTime.start();

    m_chunks = new Chunk[CHUNK_NUMBER];
    m_chunksMapping[0] = new Chunk*[CHUNK_NUMBER];
    m_chunksMapping[1] = new Chunk*[CHUNK_NUMBER];

    int layerSize = (FULL_VIEW_SIZE * FULL_VIEW_SIZE);
    for (int n = 0; n < CHUNK_NUMBER; ++n) {
        int j = div_floor(n, layerSize);

        int indexInLayer = n % layerSize;
        int i = (indexInLayer % (2 * VIEW_SIZE + 1)) - VIEW_SIZE;
        int k = (indexInLayer / (2 * VIEW_SIZE + 1)) - VIEW_SIZE;
        m_chunks[n].i = i;
        m_chunks[n].j = j;
        m_chunks[n].k = k;

        m_chunks[n].data = m_chunkBuffers + n * CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE;
        m_chunks[n].vboIndex = n;

        m_chunksMapping[0][n] = m_chunks + n;
        m_chunksMapping[1][n] = m_chunks + n;

        m_toGenerateChunkData.push_back(m_chunks + n);
        m_chunks[n].inQueue = true;
        m_chunks[n].isDirty = true;
        m_chunks[n].isLightDirty = true;
    }
    m_chunksCenter[0] = {0, 0, 0};
    m_chunksCenter[1] = {0, 0, 0};
    m_mapIndex = 0;
}

ChunkManager::~ChunkManager() {
    
    for (int i = 0; i < CHUNK_NUMBER; ++i) {
        Chunk* chunk = m_chunks + i;
        if (chunk->generated && chunk->differsFromDisk) {
            Voxel* voxel_data = chunk->data;
            SaveChunkToDisk(voxel_data, Coords{chunk->i, chunk->j, chunk->k }, chunk->onlyAir, mWorldSeed);
		}
	}

	delete[] m_chunkBuffers;
	delete[] m_tempVertexData;
	delete[] m_chunkToDraw;

	delete m_ChunkGenerator;
    delete m_LightManager;
    delete m_meshGenerator;

    delete[] m_chunks;
    delete[]m_chunksMapping[0];
    delete[]m_chunksMapping[1];
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

    QVector3D skyColor(0.53f, 0.807f, 0.92f);

	m_program->bind();
	m_program->setUniformValue("atlas", 0);
	m_program->setUniformValue("tileCount", 16);
	m_program->setUniformValue("tileSize", 16);
	m_program->setUniformValue("fogDistance", (float)(VIEW_SIZE*CHUNK_SIZE*CHUNK_SCALE)*3.0f);
	m_program->setUniformValue("fogColor", skyColor);
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
    m_waterTimerUniform = m_waterProgram->uniformLocation("time");
	m_waterProgram->bind();
	m_waterProgram->setUniformValue("atlas", 0);
	m_waterProgram->setUniformValue("tileCount", 16);
	m_waterProgram->setUniformValue("tileSize", 16);
	m_waterProgram->setUniformValue("fogDistance", (float)(VIEW_SIZE*CHUNK_SIZE*CHUNK_SCALE)*3.0f);
	m_waterProgram->setUniformValue("fogColor", skyColor);
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

		buffer->vbo = vbos[i];
		buffer->vao = new QOpenGLVertexArrayObject(gl);
		buffer->vao->create();
		buffer->vao->bind();

		gl->glBindBuffer(GL_ARRAY_BUFFER, buffer->vbo);
		gl->glEnableVertexAttribArray(m_posAttr);
		gl->glVertexAttribIPointer(m_posAttr, 1, GL_UNSIGNED_INT, 0, 0);
		buffer->vao->release();

		buffer->toUpData = nullptr; 

	}

	m_isInit = true;
	
	start();
}

void ChunkManager::destroy(GameWindow* gl) {
	
	m_needRegen = false;
    this->terminate();
    this->wait(2000);

	m_isInit = false;

	// On nettoie les ressources opengl
	for (int i = 0; i < VBO_NUMBER; ++i) {
		Buffer* buffer = m_oglBuffers + i;

		buffer->vao->destroy();
		delete buffer->vao;
		gl->glDeleteBuffers(1, &buffer->vbo);


		if (buffer->toUpData)
			delete buffer->toUpData;
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

		QVector3D camPos = gl->getCamera().getPosition();

		Coords chunkHere = GetChunkPosFromVoxelPos(GetVoxelPosFromWorldPos(camPos));

		float camX = gl->getCamera().getPosition().x();
		float camY = gl->getCamera().getPosition().y();
		float camZ = gl->getCamera().getPosition().z();

        if ((chunkHere.i != m_currentChunk.i) || (chunkHere.k != m_currentChunk.k) || m_FirstUpdate) {
            m_currentChunk = chunkHere;

            int currentMapIndex = m_mapIndex;
            int nextMapIndex = (currentMapIndex + 1) % 2;

            memset(m_chunksMapping[nextMapIndex], 0, sizeof(Chunk*) * CHUNK_NUMBER);
            m_nextFreeBuffer.clear();
            for (int i = 0; i < CHUNK_NUMBER; ++i) {
                Chunk* chunk = m_chunks + i;

                // Les chunks hors de la vue
                if ((std::abs(chunk->i - chunkHere.i) > VIEW_SIZE) ||
                    (std::abs(chunk->k - chunkHere.k) > VIEW_SIZE)) {
                    m_mutexGenerateQueue.lock();
                    m_toGenerateChunkData.removeAll(chunk);
                    chunk->inQueue = false;
                    m_mutexGenerateQueue.unlock();

                    chunk->generated = false;
                    m_nextFreeBuffer.push_back(chunk);
                } else {
                    int newIndex = getArrayIndex(chunk->i, chunk->j, chunk->k, chunkHere);
                    m_chunksMapping[nextMapIndex][newIndex] = chunk;
                }
            }
            Chunk** newChunkArray = m_chunksMapping[nextMapIndex];
            for (int i = 0; i < CHUNK_NUMBER; ++i) {
                if (newChunkArray[i] == nullptr) {
                    newChunkArray[i] = m_nextFreeBuffer.back();
                    m_nextFreeBuffer.pop_back();
                    Coords c = getChunkCoords(i, chunkHere);
                    newChunkArray[i]->i = c.i;
                    newChunkArray[i]->j = c.j;
                    newChunkArray[i]->k = c.k;

                    m_mutexGenerateQueue.lock();
                    m_toGenerateChunkData.push_back(newChunkArray[i]);
                    newChunkArray[i]->inQueue = true;
                    m_mutexGenerateQueue.unlock();
                }
            }
            m_chunksCenter[nextMapIndex] = chunkHere;
            m_mapIndex = nextMapIndex;
        }
        int remesh_count = 0;
        for (int i = 0; i < CHUNK_NUMBER; ++i) {
            Chunk* chunk = m_chunks + i;
            bool remesh = false;
            if ((chunk->isDirty || chunk->isLightDirty) && (remesh_count < MAX_REMESH_PER_UPDATE) && (chunk->vboIndex != -1) && chunk->generated){
                chunk->isDirty = false;
                chunk->isLightDirty = false;
                remesh = true;
            }

            // Regénération du mesh pour ce chunk
            if (remesh) {
                remesh_count++;
                Buffer* buffer = m_oglBuffers + chunk->vboIndex;
                Voxel *data = chunk->data;
                unsigned int totalCount = 0;
                if (data != nullptr){
                    totalCount = m_meshGenerator->generate(data, Coords{ chunk->i, chunk->j, chunk->k }, buffer, m_tempVertexData);
                }
                if (totalCount > 0) {
                    gl->glBindBuffer(GL_ARRAY_BUFFER, buffer->vbo);
                    // On alloue un buffer plus grand si besoin
                    if (totalCount > (buffer->opaqueCount + buffer->waterCount))
                        gl->glBufferData(GL_ARRAY_BUFFER, totalCount * sizeof(GLuint), 0, GL_DYNAMIC_DRAW);

                    void *dataMap = gl->glMapBufferRange(GL_ARRAY_BUFFER, 0, totalCount*sizeof(GLuint), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
                    if (dataMap != nullptr){
                        memcpy(dataMap, m_tempVertexData, totalCount*sizeof(GLuint));
                        chunk->ready = false;
                        chunk->visible = true;
                        buffer->opaqueCount = buffer->toUpOpaqueCount;
                        buffer->waterCount = buffer->toUpWaterCount;
                        buffer->draw = true;
                        chunk->ready = false;
                        chunk->visible = true;

                        remesh = false;
                    }
                    gl->glUnmapBuffer(GL_ARRAY_BUFFER);
                    gl->glBindBuffer(GL_ARRAY_BUFFER, 0);

                    m_vboToUpload = -1;
                } else {
                    buffer->draw = true;
                    chunk->ready = false;
                    chunk->visible = true;
                }
            }


            if (chunk->visible && !chunk->onlyAir) {
                m_chunkToDraw[m_chunkToDrawCount++] = chunk;
                int dx = camX - chunk->i*CHUNK_SIZE*CHUNK_SCALE;
                int dy = camY - chunk->j*CHUNK_SIZE*CHUNK_SCALE;
                int dz = camZ - chunk->k*CHUNK_SIZE*CHUNK_SCALE;
                chunk->distanceFromCamera = dx * dx + dy * dy + dz * dz;
            }
        }

		std::sort(m_chunkToDraw, m_chunkToDraw + m_chunkToDrawCount, [this](Chunk* i, Chunk* j)->bool{
			return i->distanceFromCamera < j->distanceFromCamera;
		});
		
		m_FirstUpdate = false;
	}
}

int ChunkManager::getArrayIndex(Coords chunkPos, Coords center) {
    int i = (chunkPos.i - center.i) + VIEW_SIZE;
    int k = (chunkPos.k - center.k) + VIEW_SIZE;
    return (i + ((k + (chunkPos.j * FULL_VIEW_SIZE)) * FULL_VIEW_SIZE));
}

int ChunkManager::getArrayIndex(int i, int j, int k, Coords center) {
    int newI = (i - center.i) + VIEW_SIZE;
    int newK = (k - center.k) + VIEW_SIZE;
    return (newI + ((newK + (j * FULL_VIEW_SIZE)) * FULL_VIEW_SIZE));
}

Coords ChunkManager::getChunkCoords(int index, Coords center) {
    int layerSize = (FULL_VIEW_SIZE * FULL_VIEW_SIZE);

    int j = div_floor(index, layerSize);
    int indexInLayer = index % layerSize;
    int i = (indexInLayer % FULL_VIEW_SIZE) - VIEW_SIZE + center.i;
    int k = (indexInLayer / FULL_VIEW_SIZE) - VIEW_SIZE + center.k;

    return {i, j, k};
}

void ChunkManager::draw(GameWindow* gl) {
	if (m_isInit) {
		m_program->bind();

		QMatrix4x4 mat = gl->getCamera().getViewProjMatrix();
		QMatrix4x4 scale;
		scale.scale(CHUNK_SCALE, CHUNK_SCALE, CHUNK_SCALE);
		m_program->setUniformValue(m_matrixUniform, mat * scale);
		m_atlas->bind(0);

		for (int i = m_chunkToDrawCount - 1; i >= 0; --i) {
			Chunk* chunk = m_chunkToDraw[i];
			Buffer* buffer = m_oglBuffers + chunk->vboIndex;

            if (chunk->generated && buffer->draw && buffer->opaqueCount > 0) {
				buffer->vao->bind();

				m_program->setUniformValue(m_chunkPosUniform, QVector3D(chunk->i * CHUNK_SIZE, chunk->j * CHUNK_SIZE, chunk->k * CHUNK_SIZE));
				//m_program->setUniformValue(m_lightMapUniform, 1);
				gl->glDrawArrays(GL_TRIANGLES, 0, buffer->opaqueCount);
				buffer->vao->release();
			}
		}
		glDisable(GL_CULL_FACE);
		m_waterProgram->bind();
		m_waterProgram->setUniformValue(m_waterMatrixUniform, mat * scale);
        m_waterProgram->setUniformValue(m_waterTimerUniform, (float)m_animationTime.elapsed());
		for (int i = m_chunkToDrawCount - 1; i >= 0; --i) {
			Chunk* chunk = m_chunkToDraw[i];
			Buffer* buffer = m_oglBuffers + chunk->vboIndex;
            if (chunk->generated && buffer->draw && buffer->waterCount > 0) {
				buffer->vao->bind();

				m_waterProgram->setUniformValue(m_waterChunkPosUniform, QVector3D(chunk->i * CHUNK_SIZE, chunk->j * CHUNK_SIZE, chunk->k * CHUNK_SIZE));
				gl->glDrawArrays(GL_TRIANGLES, buffer->opaqueCount, buffer->waterCount);
				buffer->vao->release();
			}
		}
		glEnable(GL_CULL_FACE);

	}
}

Voxel* ChunkManager::getBufferAdress(int index) {
	return index != -1 ? m_chunkBuffers + (index * CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE) : nullptr;
}

Chunk* ChunkManager::getChunk(Coords pos) {
    Chunk* lastChunk = m_lastChunk;
    if (lastChunk != nullptr) {
        Coords c = {lastChunk->i, lastChunk->j, lastChunk->k};
        if (c == pos)
            return lastChunk;
    }

    int mapIndex = m_mapIndex;
    Coords currentPos = m_chunksCenter[mapIndex];
    if ((std::abs(pos.i - currentPos.i) > VIEW_SIZE) ||
        (std::abs(pos.k - currentPos.k) > VIEW_SIZE)) {
        return nullptr;
    }

	if (pos.j < 0 || pos.j >= WORLD_HEIGHT)
		return nullptr;

    int i = (pos.i - currentPos.i) + VIEW_SIZE;
    int k = (pos.k - currentPos.k) + VIEW_SIZE;
    int index = i + ((k + (pos.j * FULL_VIEW_SIZE)) * FULL_VIEW_SIZE);

    m_lastChunk = m_chunksMapping[mapIndex][index];
    return m_chunksMapping[mapIndex][index];
}

Chunk* ChunkManager::getChunk(int i, int j, int k) {
	return getChunk({ i, j, k });
}

void ChunkManager::run() {

	while (m_needRegen) {
        // On génére tous les chunks requis

		if (m_toGenerateChunkData.size() > 0) {
			m_mutexGenerateQueue.lock();
			Coords here = m_currentChunk;
            std::sort(m_toGenerateChunkData.begin(), m_toGenerateChunkData.end(), [here](Chunk* c1, Chunk* c2)->bool{
				int a = (c1->i - here.i)*(c1->i - here.i) + (c1->k - here.k)*(c1->k - here.k);
				int b = (c2->i - here.i)*(c2->i - here.i) + (c2->k - here.k)*(c2->k - here.k);
				if (a == b) {
                    return c1->j < c2->j;
				}
                return a > b;
            });

			
            Chunk* newChunk = m_toGenerateChunkData.back();
			m_toGenerateChunkData.pop_back();
			m_mutexGenerateQueue.unlock();

            if (!newChunk->generated) {
                // Générer le nouveau chunk et le prendre du DD si déja présent
                Voxel* data = newChunk->data;
                if (data != nullptr){

                    bool skipGeneration = false;
                    if (ChunkExistsOnDisk(Coords{ newChunk->i, newChunk->j, newChunk->k }, mWorldSeed)){
                        skipGeneration = LoadChunkFromDisk(data, Coords{ newChunk->i, newChunk->j, newChunk->k }, &(newChunk->onlyAir), mWorldSeed);
                        newChunk->differsFromDisk = !skipGeneration;
                    }

                    QSet<Coords> modifiedChunks;

                    if(!skipGeneration){
                        newChunk->onlyAir = m_ChunkGenerator->generateChunk(data, newChunk->i, newChunk->j, newChunk->k, modifiedChunks);
                        SaveChunkToDisk(data, Coords{ newChunk->i, newChunk->j, newChunk->k }, newChunk->onlyAir, mWorldSeed);
                        newChunk->differsFromDisk = false;
                    }

                    m_LightManager->updateLighting(newChunk);
                    modifiedChunks.remove(Coords{ newChunk->i, newChunk->j, newChunk->k });
                    for (auto pos : modifiedChunks) {
                        Chunk* c = getChunk(pos);
                        if (c != nullptr){
                            if (c->generated) {
                                m_LightManager->updateLighting(c);
                                Voxel* voxel_data = c->data;
                                SaveChunkToDisk(voxel_data, Coords{ c->i, c->j, c->k }, c->onlyAir, mWorldSeed);
                                c->isDirty = true;
                                c->differsFromDisk = false;
                            }
                        }
                    }

                    newChunk->generated = true;
                    newChunk->isDirty = true;
                } else {
                    newChunk->isDirty = true;
                }
            }
            newChunk->inQueue = false;
		}

		// Eviter de faire fondre le cpu dans des boucles vides ;)
        //QThread::msleep(5);
	}
}

Voxel ChunkManager::getVoxel(int x, int y, int z, bool* loaded) {
	Voxel res = IGNORE_VOXEL;
    if (loaded != nullptr) {
        *loaded = true;
    }
    Coords c = GetChunkPosFromVoxelPos({ x, y, z });
    Chunk* chunk = getChunk(c);
	if (chunk == nullptr) {
        if (loaded != nullptr) {
            *loaded = false;
        }
		if (y >= WORLD_HEIGHT*CHUNK_SIZE)
			res._light = SUN_LIGHT;

		return res;
    }

    Voxel* voxels = chunk->data;
    if (voxels != nullptr) {
        Coords c = GetVoxelRelPos({ x, y, z });
        res = voxels[IndexVoxelRelPos(c)];
    } else if (loaded != nullptr) {
        *loaded = false;
    }
	return res;
}

Voxel ChunkManager::getVoxel(Coords c) {
	return getVoxel(c.i, c.j, c.k);
}

VoxelType ChunkManager::setVoxel(Coords c, VoxelType newType, uint light) {
	return setVoxel(c.i, c.j, c.k, newType, light);
}

VoxelType ChunkManager::setVoxel(int x, int y, int z, VoxelType newType, uint light) {
	VoxelType res = VoxelType::IGNORE_TYPE;

    Chunk* chunk = getChunk(div_floor(x, CHUNK_SIZE), div_floor(y, CHUNK_SIZE), div_floor(z, CHUNK_SIZE));
	if (chunk == nullptr)
		return res;
    Voxel* voxels = chunk->data;
    if (voxels != nullptr) {
        Coords c = GetVoxelRelPos({ x, y, z });
        int index = IndexVoxelRelPos(c);

        Voxel* v = &voxels[index];

        res = v->type;

        if (light != NO_CHANGE) {
            v->_light = (uint8)light;
        }

        v->type = newType;

        if (chunk->onlyAir && newType != VoxelType::AIR)
            chunk->onlyAir = false;
    }

	return res;
}

LightManager& ChunkManager::getLightManager() {
	return *m_LightManager;
}

VoxelType ChunkManager::placeVoxel(Coords pos, VoxelType type) {
	
	QSet<Coords> modifiedChunks;

	VoxelType oldType = getVoxel(pos).type;

	m_LightManager->placeVoxel(pos, type, modifiedChunks);

	Chunk* thisChunk = getChunk(GetChunkPosFromVoxelPos(pos));
	
	thisChunk->differsFromDisk = true;

	for (auto coords : modifiedChunks) {
		Chunk* chunk = getChunk(coords);
		if (chunk != nullptr) {
			chunk->inQueue = true;
            chunk->isLightDirty = true;
		}
	}
	if (thisChunk != nullptr) {
		thisChunk->inQueue = true;
		thisChunk->isLightDirty = true;
		thisChunk->isDirty = true;

		if(thisChunk->onlyAir && type != VoxelType::AIR)
			thisChunk->onlyAir = false;
	}

	return oldType;
}

void ChunkManager::removeVoxel(Coords pos) {

	QSet<Coords> modifiedChunks;
	m_LightManager->removeVoxel(pos, modifiedChunks);

	Chunk* thisChunk = getChunk(GetChunkPosFromVoxelPos(pos));
	thisChunk->differsFromDisk = true;

	for (auto coords : modifiedChunks) {
		Chunk* chunk = getChunk(coords);
		if (chunk != nullptr) {
			chunk->inQueue = true;
			chunk->isLightDirty = true;
		}
	}

	if (thisChunk != nullptr) {
		thisChunk->inQueue = true;
		thisChunk->isLightDirty = true;
		thisChunk->isDirty = true;
	}
}
