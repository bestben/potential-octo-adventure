#include "chunkmanager.h"
#include "gamewindow.h"
#include "save.h"

#include <iostream>

#include <QtGui/QOpenGLVertexArrayObject>
#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QOpenGLTexture>
#include <QtCore/QSet>

ChunkManager::ChunkManager() : m_isInit{ false },
m_chunkBuffers{ nullptr }, m_oglBuffers{ nullptr }, m_FirstUpdate{ true }{
	m_LightManager = new LightManager(this);
	m_meshGenerator = new MeshGenerator(this);
	m_ChunkGenerator = new ChunkGenerator(this);
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

    m_lastChunk = nullptr;

	m_currentChunk = { 0, -1, 0 };
	m_animationTime.start();
}

ChunkManager::~ChunkManager() {
	m_mutexChunkManagerList.lock();

	for (auto* chunk : m_ChunkMap) {

		if (chunk->chunkBufferIndex != -1 && chunk->generated && chunk->differsFromDisk){
			Voxel* voxel_data = getBufferAdress(chunk->chunkBufferIndex);
			SaveChunkToDisk(voxel_data, Coords{ chunk->i, chunk->j, chunk->k }, chunk->onlyAir);
		}

		delete chunk;
	}

	m_mutexChunkManagerList.unlock();
	delete[] m_chunkBuffers;
	delete[] m_availableChunkData;
	delete[] m_inUseChunkData;
	delete[] m_availableBuffer;
	delete[] m_tempVertexData;
	delete[] m_chunkToDraw;

	delete m_ChunkGenerator;
    delete m_LightManager;
    delete m_meshGenerator;
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

	QVector3D skyColor(0.53f, 0.807f, 0.92);

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
	m_chunkDataLeft = CHUNK_NUMBER;
	m_vboLeft = VBO_NUMBER;

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

		uint16 remesh_count = 0;

		for (auto ite = m_ChunkMap.begin(); ite != m_ChunkMap.end();) {
			auto chunk = *ite;
			if (chunk == nullptr){ ++ite; continue; }

			// Les chunks hors de la vue 
			if ((std::abs(chunk->i - m_currentChunk.i) > VIEW_SIZE) ||
				(std::abs(chunk->k - m_currentChunk.k) > VIEW_SIZE)) {
				chunk->visible = false;
				m_mutexGenerateQueue.lock();
				m_toGenerateChunkData.removeAll(chunk);
				chunk->inQueue = false;
				m_mutexGenerateQueue.unlock();
				// Si nombre de buffers libres est sous le seuil, on libère de la mémoire
				if (m_vboLeft <= FREE_BUFFERS_THRESHOLD || m_chunkDataLeft <= FREE_BUFFERS_THRESHOLD){
					if (chunk->chunkBufferIndex != -1) {
						m_availableChunkData[chunk->chunkBufferIndex] = true;
						chunk->chunkBufferIndex = -1;
						m_chunkDataLeft++;
					}
					if (chunk->vboIndex != -1) {
						m_oglBuffers[chunk->vboIndex].draw = false;
						m_availableBuffer[chunk->vboIndex] = true;
						chunk->vboIndex = -1;
						m_vboLeft++;
					}
					chunk->generated = false;
					
					m_mutexChunkManagerList.lock();
					ite = m_ChunkMap.erase(ite);
					m_mutexChunkManagerList.unlock();
				}
				else{
					m_mutexChunkManagerList.lock();
					++ite;
					m_mutexChunkManagerList.unlock();
				}
			}
			else {
				bool remesh = false;
				if ((chunk->isDirty || chunk->isLightDirty) && remesh_count < MAX_REMESH_PER_UPDATE && chunk->vboIndex != -1 && chunk->chunkBufferIndex != -1 && chunk->generated){
					chunk->isDirty = false;
					chunk->isLightDirty = false;
					remesh = true;
				}

				// Regénération du mesh pour ce chunk
				if (remesh) {
					remesh_count++;
					Buffer* buffer = m_oglBuffers + chunk->vboIndex;
					Voxel *data = getBufferAdress(chunk->chunkBufferIndex);
					unsigned int totalCount = 0;
					if (data != nullptr){
						totalCount = m_meshGenerator->generate(data, Coords{ chunk->i, chunk->j, chunk->k }, buffer, m_tempVertexData);
					}
					if (totalCount > 0){
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


				if (chunk->vboIndex != -1 && chunk->visible && !chunk->onlyAir) {
					m_chunkToDraw[m_chunkToDrawCount++] = chunk;
					int dx = camX - chunk->i*CHUNK_SIZE*CHUNK_SCALE;
					int dy = camY - chunk->j*CHUNK_SIZE*CHUNK_SCALE;
					int dz = camZ - chunk->k*CHUNK_SIZE*CHUNK_SCALE;
					chunk->distanceFromCamera = dx * dx + dy * dy + dz * dz;
				}
				m_mutexChunkManagerList.lock();
				++ite;
				m_mutexChunkManagerList.unlock();
			}
			// TODO: Save le chunk sur le DD pour réutilisation
			//++ite;
		}
		//m_mutexChunkManagerList.unlock();

		// Si on change de chunk on demande des nouveaux chunks
		if ((chunkHere.i != m_currentChunk.i) || (chunkHere.k != m_currentChunk.k) || m_FirstUpdate) {
			m_currentChunk = chunkHere;
			requestChunks();
		}


		std::sort(m_chunkToDraw, m_chunkToDraw + m_chunkToDrawCount, [this](Chunk* i, Chunk* j)->bool{
			return i->distanceFromCamera < j->distanceFromCamera;
		});

		

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
		//m_mutexChunkManagerList.lock();

		for (int i = m_chunkToDrawCount - 1; i >= 0; --i) {
			Chunk* chunk = m_chunkToDraw[i];
			Buffer* buffer = m_oglBuffers + chunk->vboIndex;

			if (buffer->draw && buffer->opaqueCount > 0) {
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
		m_waterProgram->setUniformValue("time", (float)m_animationTime.elapsed());
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
		glEnable(GL_CULL_FACE);
		//m_mutexChunkManagerList.unlock();

	}
}

void ChunkManager::requestChunks() {

	//m_mutexChunkManagerList.lock();
	for (int i = 0; i <= VIEW_SIZE; ++i) {
		for (int k = 0; k <= VIEW_SIZE; ++k) {
			for (int j = WORLD_HEIGHT-1; j >= 0; --j) {
				checkChunk({ m_currentChunk.i + i, j, m_currentChunk.k + k });
				checkChunk({ m_currentChunk.i - i, j, m_currentChunk.k + k });
				checkChunk({ m_currentChunk.i + i, j, m_currentChunk.k - k });
				checkChunk({ m_currentChunk.i - i, j, m_currentChunk.k - k });
			}
		}
	}
	//m_mutexChunkManagerList.unlock();
}

void ChunkManager::checkChunk(Coords pos) {

	Chunk* chunk = getChunk(pos);

	if (chunk == nullptr) {
		chunk = new Chunk();
		chunk->i = pos.i;
		chunk->j = pos.j;
		chunk->k = pos.k;

		m_mutexChunkManagerList.lock();
		m_ChunkMap[pos] = chunk;
		m_mutexChunkManagerList.unlock();
	}

	if (!chunk->generated && !chunk->inQueue) {
		chunk->inQueue = true;
		m_mutexGenerateQueue.lock();
		m_toGenerateChunkData.push_back(chunk);
		m_mutexGenerateQueue.unlock();
	}
	chunk->visible = true;
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

	if (pos.j < 0 || pos.j >= WORLD_HEIGHT)
		return nullptr;

    m_mutexChunkManagerList.lock();
	auto it = m_ChunkMap.find(pos);
	auto end = m_ChunkMap.end();
    m_mutexChunkManagerList.unlock();
	if (it == end) {
		return nullptr;
	}

    m_lastChunkId = pos;
	m_lastChunk = *it;
    return *it;
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
				int a = (c1->i - here.i)*(c1->i - here.i) + (c1->j - here.j)*(c1->j - here.j) + (c1->k - here.k)*(c1->k - here.k);
				int b = (c2->i - here.i)*(c2->i - here.i) + (c2->j - here.j)*(c2->j - here.j) + (c2->k - here.k)*(c2->k - here.k);
				return a>b;
			});

			
			auto* newChunk = m_toGenerateChunkData.back();
			m_toGenerateChunkData.pop_back();
			m_mutexGenerateQueue.unlock();
			
			
			int bufferIndex = newChunk->chunkBufferIndex;
			int vboIndex = newChunk->vboIndex;

			bool newBuffer = false;
			bool newVBO = false;

			if (bufferIndex == -1){
				bufferIndex = seekFreeChunkData();
				newBuffer = true;
			}
			if (vboIndex == -1){
				vboIndex = seekFreeBuffer();
				newVBO = true;
			}

			if (bufferIndex != -1 && vboIndex != -1) {
				
				if(newBuffer)
					m_chunkDataLeft--;
				if(newVBO)
					m_vboLeft--;

				m_availableChunkData[bufferIndex] = false;
				m_availableBuffer[vboIndex] = false;

				newChunk->vboIndex = vboIndex;
				newChunk->chunkBufferIndex = bufferIndex;


				if (!newChunk->generated){
					// Générer le nouveau chunk et le prendre du DD si déja présent
					Voxel* data = getBufferAdress(bufferIndex);
					if (data != nullptr){

						bool skipGeneration = false;
						if(ChunkExistsOnDisk(Coords{newChunk->i, newChunk->j, newChunk->k})){
							skipGeneration = LoadChunkFromDisk(data, Coords{ newChunk->i, newChunk->j, newChunk->k }, &(newChunk->onlyAir));
							newChunk->differsFromDisk = !skipGeneration;
						}

						QSet<Coords> modifiedChunks;

						if(!skipGeneration){
							newChunk->onlyAir = m_ChunkGenerator->generateChunk(data, newChunk->i, newChunk->j, newChunk->k, modifiedChunks);
							SaveChunkToDisk(data, Coords{newChunk->i, newChunk->j, newChunk->k}, newChunk->onlyAir);
							newChunk->differsFromDisk = false;
						}

                        m_LightManager->updateLighting(newChunk);
						modifiedChunks.remove(Coords{ newChunk->i, newChunk->j, newChunk->k });
						for (auto pos : modifiedChunks) {
							auto* c = getChunk(pos);
							if (c != nullptr){
								if (c->chunkBufferIndex != -1 && c->generated){
									m_LightManager->updateLighting(c);
									Voxel* voxel_data = getBufferAdress(c->chunkBufferIndex);
									SaveChunkToDisk(voxel_data, Coords{ c->i, c->j, c->k }, c->onlyAir);
									c->isDirty = true;
									c->differsFromDisk = false;
								}
							}
						}

						newChunk->generated = true;
						newChunk->isDirty = true;
					}
					else {
						newChunk->isDirty = true;
					}
				}
				newChunk->inQueue = false;
			}
			else {
				// Plus assez de blocs libres
				m_mutexGenerateQueue.lock();
				m_toGenerateChunkData.push_back(newChunk);
				m_mutexGenerateQueue.unlock();
				
				#ifdef QT_DEBUG
				std::cout << "Plus assez de blocs libre pour charger un chunk" << std::endl;
				#endif
			}

		}



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

Voxel ChunkManager::getVoxel(int x, int y, int z, bool* loaded) {
	Voxel res = IGNORE_VOXEL;
    if (loaded != nullptr) {
        *loaded = true;
    }

	Chunk* chunk = getChunk(GetChunkPosFromVoxelPos({ x, y, z }));
	if (chunk == nullptr) {
        if (loaded != nullptr) {
            *loaded = false;
        }
		if (y >= WORLD_HEIGHT*CHUNK_SIZE)
			res._light = SUN_LIGHT;

		return res;
    }

	if (chunk->chunkBufferIndex != -1) {
		Voxel* voxels = getBufferAdress(chunk->chunkBufferIndex);
		if (voxels != nullptr) {
			Coords c = GetVoxelRelPos({ x, y, z });
			res = voxels[IndexVoxelRelPos(c)];
		} else if (loaded != nullptr) {
            *loaded = false;
        }
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
	if (chunk->chunkBufferIndex != -1) {
		Voxel* voxels = getBufferAdress(chunk->chunkBufferIndex);
		if (voxels != nullptr) {
			Coords c = GetVoxelRelPos({ x, y, z });
			int index = IndexVoxelRelPos(c);

			Voxel* v = &voxels[index];

			bool changed = false;
			res = v->type;

			if (light != NO_CHANGE){
                v->_light = (uint8)light;
			}

            v->type = newType;

			if (chunk->onlyAir && newType != VoxelType::AIR)
				chunk->onlyAir = false;
			
		}
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
			//m_toGenerateChunkData.push_front(chunk);
			chunk->isLightDirty = true;
		}
	}
	if (thisChunk != nullptr) {
		thisChunk->inQueue = true;
		//m_toGenerateChunkData.push_front(thisChunk);
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
			//m_toGenerateChunkData.push_front(chunk);
			chunk->isLightDirty = true;
		}
	}

	if (thisChunk != nullptr) {
		thisChunk->inQueue = true;
		//m_toGenerateChunkData.push_front(thisChunk);
		thisChunk->isLightDirty = true;
		thisChunk->isDirty = true;
	}
}
