#include "chunkmanager.h"

#include "gamewindow.h"

#include <iostream>

#include <QtGui/QOpenGLVertexArrayObject>
#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QOpenGLTexture>

ChunkManager::ChunkManager() : m_isInit{ false }, m_chunkBuffers{ nullptr },
m_oglBuffers{ nullptr }, m_ChunkGenerator(), m_FirstUpdate{ true }{
	m_LightManager = new LightManager(this);
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
	m_animationTime.start();
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
	m_lightMapUniform = m_program->uniformLocation("lightMap");

	m_program->bind();
	m_program->setUniformValue("atlas", 0);
	m_program->setUniformValue("lightMap", 1);

	m_program->setUniformValue("lightMapXP", 2);
	m_program->setUniformValue("lightMapXM", 3);

	m_program->setUniformValue("lightMapYP", 4);
	m_program->setUniformValue("lightMapYM", 5);

	m_program->setUniformValue("lightMapZP", 6);
	m_program->setUniformValue("lightMapZM", 7);


	m_program->setUniformValue("tileCount", 16);
	m_program->setUniformValue("tileSize", 16);
	m_program->setUniformValue("fogDistance", (float)(VIEW_SIZE*CHUNK_SIZE*CHUNK_SCALE));
	// TODO: Utiliser la couleur du ciel à la place
	m_program->setUniformValue("fogColor", QVector4D(.5f, .5f, .5f, 1.0f));
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

	m_waterProgram->setUniformValue("lightMapXP", 2);
	m_waterProgram->setUniformValue("lightMapXM", 3);

	m_waterProgram->setUniformValue("lightMapYP", 4);
	m_waterProgram->setUniformValue("lightMapYM", 5);

	m_waterProgram->setUniformValue("lightMapZP", 6);
	m_waterProgram->setUniformValue("lightMapZM", 7);

	m_waterProgram->setUniformValue("lightMap", 1);
	m_waterProgram->setUniformValue("tileCount", 16);
	m_waterProgram->setUniformValue("tileSize", 16);
	m_waterProgram->setUniformValue("fogDistance", (float)(VIEW_SIZE*CHUNK_SIZE*CHUNK_SCALE));
	// TODO: Utiliser la couleur du ciel à la place
	m_waterProgram->setUniformValue("fogColor", QVector4D(.5f, .5f, .5f, 1.0f));
	m_waterProgram->release();

	m_atlas = new QOpenGLTexture(QImage(":/atlas.png"));
	m_atlas->setMagnificationFilter(QOpenGLTexture::Nearest);

	GLuint vbos[VBO_NUMBER];
	gl->glGenBuffers(VBO_NUMBER, vbos);

	GLuint vbos_light[VBO_NUMBER];
	gl->glGenBuffers(VBO_NUMBER, vbos_light);

	uint16 fullbright[CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE];

	for (int i = 0; i < CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE; ++i)
		fullbright[i] = 0;


	m_oglBuffers = new Buffer[VBO_NUMBER];
	for (int i = 0; i < VBO_NUMBER; ++i) {
		Buffer* buffer = m_oglBuffers + i;
		buffer->opaqueCount = 0;
		buffer->waterCount = 0;
		buffer->draw = false;

		buffer->texture_light = new QOpenGLTexture(QOpenGLTexture::TargetBuffer);
		buffer->texture_light->create();

		buffer->vbo_light = vbos_light[i];

		buffer->vbo = vbos[i];
		buffer->vao = new QOpenGLVertexArrayObject(gl);
		buffer->vao->create();
		buffer->vao->bind();

		gl->glBindBuffer(GL_TEXTURE_BUFFER, buffer->vbo_light);
		gl->glBufferData(GL_TEXTURE_BUFFER, sizeof(fullbright), fullbright, GL_DYNAMIC_DRAW);

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
		buffer->texture_light->destroy();
		delete buffer->texture_light;
		buffer->vao->destroy();
		delete buffer->vao;
		gl->glDeleteBuffers(1, &buffer->vbo);
		gl->glDeleteBuffers(1, &buffer->vbo_light);
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
			}
			else {
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
			Buffer& buffer = m_oglBuffers[m_vboToUpload];
			m_canUploadMesh = false;
			gl->glBindBuffer(GL_ARRAY_BUFFER, buffer.vbo);
			// On alloue un buffer plus grand si besoin
			if (m_countToUpload > (buffer.opaqueCount + buffer.waterCount))
				gl->glBufferData(GL_ARRAY_BUFFER, m_countToUpload * sizeof(GLuint), 0, GL_DYNAMIC_DRAW);

			void *data = gl->glMapBufferRange(GL_ARRAY_BUFFER, 0, m_countToUpload*sizeof(GLuint), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
			memcpy(data, m_tempVertexData, m_countToUpload*sizeof(GLuint));
			gl->glUnmapBuffer(GL_ARRAY_BUFFER);
			gl->glBindBuffer(GL_ARRAY_BUFFER, 0);
			buffer.opaqueCount = m_tempBufferToUpload.opaqueCount;
			buffer.waterCount = m_tempBufferToUpload.waterCount;
			buffer.draw = true;
			m_vboToUpload = -1;

			uploadLightMap(gl, m_chunkToUpload);

		}
		m_canGenerateMesh = true;

		m_LightManager->update(gl);

		float camX = gl->getCamera().getPosition().x();
		float camY = gl->getCamera().getPosition().y();
		float camZ = gl->getCamera().getPosition().z();
		for (auto* chunk : m_ChunkMap) {
			if (chunk == nullptr)
				continue;
			if (chunk->vboIndex != -1 && chunk->visible) {
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

				buffer->texture_light->bind(1);
				gl->glTexBuffer(GL_TEXTURE_BUFFER, GL_R16UI, buffer->vbo_light);

				Buffer* sideBuffer;
				int nb = 2;

				BIND_LIGHT_MAP_SIDE(chunkXP, nb++)
				BIND_LIGHT_MAP_SIDE(chunkXM, nb++)
				BIND_LIGHT_MAP_SIDE(chunkYP, nb++)
				BIND_LIGHT_MAP_SIDE(chunkYM, nb++)
				BIND_LIGHT_MAP_SIDE(chunkZP, nb++)
				BIND_LIGHT_MAP_SIDE(chunkZM, nb++)

				m_program->setUniformValue(m_chunkPosUniform, QVector3D(chunk->i * CHUNK_SIZE, chunk->j * CHUNK_SIZE, chunk->k * CHUNK_SIZE));
				//m_program->setUniformValue(m_lightMapUniform, 1);
				gl->glDrawArrays(GL_TRIANGLES, 0, buffer->opaqueCount);
				buffer->vao->release();

				gl->glBindTexture(GL_TEXTURE_BUFFER, 0);
			}
		}
		//glDisable(GL_CULL_FACE);
		m_waterProgram->bind();
		m_waterProgram->setUniformValue(m_waterMatrixUniform, mat * scale);
		m_waterProgram->setUniformValue("time", (float)m_animationTime.elapsed());
		for (int i = m_chunkToDrawCount - 1; i >= 0; --i) {
			Chunk* chunk = m_chunkToDraw[i];
			Buffer* buffer = m_oglBuffers + chunk->vboIndex;
			if (buffer->draw && buffer->waterCount > 0) {
				buffer->vao->bind();

				buffer->texture_light->bind(1);
				gl->glTexBuffer(GL_TEXTURE_BUFFER, GL_R16UI, buffer->vbo_light);

				Buffer* sideBuffer;
				int nb = 2;
				BIND_LIGHT_MAP_SIDE(chunkXP, nb++)
				BIND_LIGHT_MAP_SIDE(chunkXM, nb++)
				BIND_LIGHT_MAP_SIDE(chunkYP, nb++)
				BIND_LIGHT_MAP_SIDE(chunkYM, nb++)
				BIND_LIGHT_MAP_SIDE(chunkZP, nb++)
				BIND_LIGHT_MAP_SIDE(chunkZM, nb++)

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
		if ((chunk->chunkBufferIndex == -1 || chunk->vboIndex == -1) && !chunk->inQueue) {
			chunk->inQueue = true;
			m_toGenerateChunkData.push_back(chunk);
		}
		else {
			chunk->visible = true;
		}


	}
	else {
		// Chunk does not exist
		Chunk* chunk = new Chunk();
		chunk->i = tuple.i;
		chunk->j = tuple.j;
		chunk->k = tuple.k;

		chunk->chunkXP = nullptr;
		chunk->chunkXM = nullptr;
		chunk->chunkYP = nullptr;
		chunk->chunkYM = nullptr;
		chunk->chunkZP = nullptr;
		chunk->chunkZM = nullptr;
		chunk->inQueue = false;

		chunk->chunkBufferIndex = -1;
		chunk->vboIndex = -1;
		chunk->visible = false;
		m_ChunkMap[tuple] = chunk;
		m_toGenerateChunkData.push_back(chunk);
	}
}

void ChunkManager::findNeighbors(Chunk* chunk) {
	if (chunk->chunkBufferIndex == -1) return;
	Coords tuple = { chunk->i, chunk->j, chunk->k };
	if (!chunk->chunkXM){
		chunk->chunkXM = getChunk(tuple + Coords{ -1, 0, 0 });
		if (chunk->chunkXM)
			chunk->chunkXM->chunkXP = chunk;
	}
	if (!chunk->chunkXP){
		chunk->chunkXP = getChunk(tuple + Coords{ 1, 0, 0 });
		if (chunk->chunkXP)
			chunk->chunkXP->chunkXM = chunk;
	}

	if (!chunk->chunkYM){
		chunk->chunkYM = tuple.j>0 ? getChunk(tuple + Coords{ 0, -1, 0 }) : nullptr;
		if (chunk->chunkYM)
			chunk->chunkYM->chunkYP = chunk;
	}
	if (!chunk->chunkYP){
		chunk->chunkYP = tuple.j<6 ? getChunk(tuple + Coords{ 0, 1, 0 }) : nullptr;
		if (chunk->chunkYP)
			chunk->chunkYP->chunkYM = chunk;
	}

	if (!chunk->chunkZM){
		chunk->chunkZM = getChunk(tuple + Coords{ 0, 0, -1 });
		if (chunk->chunkZM)
			chunk->chunkZM->chunkZP = chunk;
	}
	if (!chunk->chunkZP){
		chunk->chunkZP = getChunk(tuple + Coords{ 0, 0, 1 });
		if (chunk->chunkZP)
			chunk->chunkZP->chunkZM = chunk;
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
	return index != -1 ? m_chunkBuffers + (index * CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE) : nullptr;
}


Chunk* ChunkManager::getChunk(Coords pos) {
	auto it = m_ChunkMap.find(pos);
	if (it == m_ChunkMap.end()) {
		//return nullptr;
		Chunk *chunk = new Chunk();
		chunk->i = pos.i;
		chunk->j = pos.j;
		chunk->k = pos.k;
		chunk->chunkBufferIndex = -1;
		chunk->vboIndex = -1;
		chunk->visible = false;

		chunk->chunkXP = nullptr;
		chunk->chunkXM = nullptr;
		chunk->chunkYP = nullptr;
		chunk->chunkYM = nullptr;
		chunk->chunkZP = nullptr;
		chunk->chunkZM = nullptr;

		chunk->inQueue = false;

		m_ChunkMap[pos] = chunk;
		return m_ChunkMap[pos];
	}
	return *it;
}

Chunk* ChunkManager::getChunk(int i, int j, int k) {
	return getChunk({ i, j, k });
}

void ChunkManager::run() {

	while (m_needRegen) {
		// On génére tous les chunks requis
		

		if (m_toGenerateChunkData.size() > 0) {
			// Le chunk à créer
			m_mutexChunkManagerList.lock();
			auto* newChunk = m_toGenerateChunkData.front();
			m_toGenerateChunkData.pop_front();
			// On enlève les doubles
			m_toGenerateChunkData.removeAll(newChunk);

			int bufferIndex = seekFreeChunkData();
			int vboIndex = seekFreeBuffer();

			if (bufferIndex != -1 && vboIndex != -1) {
				
			

				newChunk->vboIndex = vboIndex;

				m_availableChunkData[bufferIndex] = false;
				m_availableBuffer[vboIndex] = false;
				m_mutexChunkManagerList.unlock();

				// TODO Générer le nouveau chunk et le prendre du DD si déja présent
				Voxel* data = getBufferAdress(bufferIndex);
				m_ChunkGenerator.generateChunk(data, newChunk->i, newChunk->j, newChunk->k);
				
				

				m_mutexChunkManagerList.lock();

				newChunk->chunkBufferIndex = bufferIndex;
				newChunk->inQueue = false;
				findNeighbors(newChunk);
				
				m_LightManager->processChunk(newChunk);
				m_toGenerateBuffer.push_back(newChunk);

			}
			else {
				// Plus assez de blocs libres
				m_toGenerateChunkData.push_back(newChunk);
#ifdef QT_DEBUG
				std::cout << "Plus assez de blocs libre pour charger un chunk" << std::endl;
#endif
			}
			m_mutexChunkManagerList.unlock();
		}

		// On transforme tous les chunks en meshs.
		if (m_toGenerateBuffer.size() > 0 && m_canGenerateMesh) {
			m_mutexChunkManagerList.lock();
			m_canGenerateMesh = false;

			// Le chunk à créer
			auto* newChunk = m_toGenerateBuffer.front();
			m_toGenerateBuffer.pop_front();

			if (newChunk->chunkBufferIndex != -1) {
				if (newChunk->vboIndex != -1) {

					m_vboToUpload = newChunk->vboIndex;
					m_availableBuffer[m_vboToUpload] = false;
					m_oglBuffers[m_vboToUpload].draw = true;

					m_chunkToUpload = newChunk;
					Voxel *data = getBufferAdress(newChunk->chunkBufferIndex);
					m_countToUpload = m_meshGenerator.generate(data, &m_tempBufferToUpload, m_tempVertexData);

					m_canUploadMesh = true;
					newChunk->visible = true;
				}
				else {
					// Plus assez de vbo libres
					// Ne devrait jamais arriver
					m_toGenerateBuffer.push_back(newChunk);
#ifdef QT_DEBUG
					std::cout << "Plus assez de vbo libre pour charger un chunk" << std::endl;
#endif
				}
			}
			else {
#ifdef QT_DEBUG
				std::cout << "Impossible de generer un mesh à partir d'un chunk non charge" << std::endl;
#endif
			}
			m_mutexChunkManagerList.unlock();
		}

		// Eviter de faire fondre le cpu dans des boucles vides ;)
		QThread::msleep(33);
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

	Chunk& chunk = *getChunk(voxelGetChunk({ x, y, z }));
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

	Chunk& chunk = *getChunk(div_floor(x, CHUNK_SIZE), div_floor(y, CHUNK_SIZE), div_floor(z, CHUNK_SIZE));
	if (chunk.chunkBufferIndex != -1) {
		Voxel* voxels = getBufferAdress(chunk.chunkBufferIndex);
		if (voxels != nullptr) {
			Coords c = voxelCoordsToChunkCoords({ x, y, z });
			res = voxels[c.i + CHUNK_SIZE * (c.j + CHUNK_SIZE * c.k)].type;
			voxels[c.i + CHUNK_SIZE * (c.j + CHUNK_SIZE * c.k)].type = newType;
			// TODO: Propager la lumière correctement
			// On lance la regen du mesh
			m_mutexChunkManagerList.lock();
			// TODO: push_front à la place pour assurer une mise à jour rapide ?
			m_toGenerateBuffer.push_back(&chunk);
			m_mutexChunkManagerList.unlock();
		}
	}
	return res;
}


void ChunkManager::uploadLightMap(GameWindow* gl, Chunk *chunk) {

	Buffer* buffer = m_oglBuffers + chunk->vboIndex;

	uint16 lightmap[CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE];
	Voxel *chunkData = getBufferAdress(chunk->chunkBufferIndex);

	for (int k = 0; k < CHUNK_SIZE; ++k) {
		for (int j = 0; j < CHUNK_SIZE; ++j) {
			for (int i = 0; i < CHUNK_SIZE; ++i) {
				int index = getIndexInChunkData({ i, j, k });
				//TODO: Bit packing instead
				lightmap[index] = chunkData[index].torchLight + chunkData[index].sunLight;
			}
		}
	}

	gl->glBindBuffer(GL_TEXTURE_BUFFER, buffer->vbo_light);
	void *data = gl->glMapBufferRange(GL_TEXTURE_BUFFER, 0, CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE*sizeof(uint16), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
	memcpy(data, lightmap, CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE*sizeof(uint16));
	gl->glUnmapBuffer(GL_TEXTURE_BUFFER);

	chunk->isDirty = false;

}

LightManager& ChunkManager::getLightManager() {
	return *m_LightManager;
}