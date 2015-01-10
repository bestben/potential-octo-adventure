#include "chunkmanager.h"
#include "gamewindow.h"
#include "save.h"

#include <iostream>

#include <QtGui/QOpenGLVertexArrayObject>
#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QOpenGLTexture>

ChunkManager::ChunkManager() : m_isInit{ false }, m_chunkWorker{this}, m_meshWorker{this}, m_chunkBuffers{ nullptr },
                            m_oglBuffers{ nullptr }, m_FirstUpdate{ true }{
	m_LightManager = new LightManager(this);
	m_chunkBuffers = new Voxel[CHUNK_NUMBER * CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE];
	memset(m_chunkBuffers, 0, CHUNK_NUMBER * CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE * sizeof(Voxel));

	m_chunkToDrawCount = 0;
    m_chunkToRecycleCount = 0;
    m_chunkToGenerateCount = 0;
    m_chunkToDraw = new Chunk*[CHUNK_NUMBER];
    m_chunkToRecycle = new Chunk*[CHUNK_NUMBER];
    m_chunkToGenerate = new Chunk*[CHUNK_NUMBER];
    for (int i = 0; i < CHUNK_NUMBER; ++i) {
		m_chunkToDraw[i] = nullptr;
        m_chunkToRecycle[i] = nullptr;
        m_chunkToGenerate[i] = nullptr;
	}
    m_chunkCommandsBuffer = new ChunkWorkerCommand[CHUNK_NUMBER];

    m_nextFreeBuffers.reserve(VBO_NUMBER);
    for (int i = 0; i < VBO_NUMBER; ++i) {
        m_nextFreeBuffers.push_back(i);
    }

    m_currentChunk = { 0, -1, 0 };
    m_chunkDataLeft = CHUNK_NUMBER;

    m_chunkWorker.setCommandQueue(&m_chunkCommands);
    m_chunkWorker.setOutputCommandQueue(&m_meshCommands);
    m_meshWorker.setCommandQueue(&m_meshCommands);

    m_chunkWorker.start();
    m_meshWorker.start();

	m_animationTime.start();
}

ChunkManager::~ChunkManager() {

    m_chunkWorker.requestStop();
    m_chunkCommands.unlock();
    m_chunkWorker.terminate();
    m_chunkWorker.wait(2000);

    m_meshWorker.requestStop();
    m_meshCommands.unlock();
    m_meshWorker.terminate();
    m_meshWorker.wait(2000);

    for (std::pair<Coords, Chunk*> c : m_ChunkMap) {
        delete c.second;
    }
    delete[] m_chunkCommandsBuffer;
	delete[] m_chunkBuffers;
    delete[] m_chunkToGenerate;
    delete[] m_chunkToRecycle;
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
	m_program->setUniformValue("fogDistance", (float)(VIEW_SIZE*CHUNK_SIZE*CHUNK_SCALE)*2.0f);
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
	m_waterProgram->setUniformValue("tileCount", 16);
	m_waterProgram->setUniformValue("tileSize", 16);
	m_waterProgram->setUniformValue("fogDistance", (float)(VIEW_SIZE*CHUNK_SIZE*CHUNK_SCALE)*2.0f);
	// TODO: Utiliser la couleur du ciel à la place
	m_waterProgram->setUniformValue("fogColor", QVector4D(.5f, .5f, .5f, 1.0f));
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
        buffer->draw = true;

		buffer->vbo = vbos[i];
		buffer->vao = new QOpenGLVertexArrayObject(gl);
		buffer->vao->create();
		buffer->vao->bind();

		gl->glBindBuffer(GL_ARRAY_BUFFER, buffer->vbo);
		gl->glEnableVertexAttribArray(m_posAttr);
		gl->glVertexAttribIPointer(m_posAttr, 1, GL_UNSIGNED_INT, 0, 0);
		buffer->vao->release();

        buffer->loaded = false;
	}
    m_meshWorker.setBuffers(m_oglBuffers);

	m_isInit = true;
}

void ChunkManager::destroy(GameWindow* gl) {
    // On supprime les threads

	m_isInit = false;

	// On nettoie les ressources opengl
	for (int i = 0; i < VBO_NUMBER; ++i) {
		Buffer* buffer = m_oglBuffers + i;

		buffer->vao->destroy();
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
	if (m_isInit) {
        // On upload les buffers chargés
        m_meshWorker.uploadBuffers(gl);

		QVector3D camPos = gl->getCamera().getPosition();

		Coords chunkHere = GetChunkPosFromVoxelPos(GetVoxelPosFromWorldPos(camPos));

		float camX = gl->getCamera().getPosition().x();
		float camY = gl->getCamera().getPosition().y();
		float camZ = gl->getCamera().getPosition().z();

        // Le joueur a changé de position -> on recalcule tout
        if ((chunkHere.i != m_currentChunk.i) || (chunkHere.k != m_currentChunk.k)) {
            m_currentChunk = chunkHere;
            // On vide les commandes
            m_chunkCommands.reset();
            m_meshCommands.reset();
            m_chunkWorker.reset();

            while (!m_chunkWorker.isWaiting() || !m_meshWorker.isWaiting()) {
                // On attend que les autres threads aient fini leur commande en cours
            }

            m_chunkToRecycleCount = 0;
            m_chunkToGenerateCount = 0;
            m_chunkToDrawCount = 0;
            // On reset tous les chunks qui étaient en cours de chargement
            for (auto& p : m_ChunkMap) {
                if ((p.second != nullptr) && (p.second->state == CHUNK_LOADING)) {
                    p.second->state = CHUNK_NOT_LOADED;
                }
            }

            // On ajoute tous les chunks dont on a besoin dans la map
            m_mutexChunkManagerList.lock();
            requestChunks();
            m_mutexChunkManagerList.unlock();

            // Pour tous les chunks
            for (auto ite = m_ChunkMap.begin(); ite != m_ChunkMap.end();) {
                Chunk* chunk = ite->second;
                if (chunk == nullptr) {
                    m_mutexChunkManagerList.lock();
                    ite = m_ChunkMap.erase(ite);
                    m_mutexChunkManagerList.unlock();
                    continue;
                }
                // Chunk hors de la vue
                if ((std::abs(chunk->i - m_currentChunk.i) > VIEW_SIZE) ||
                    (std::abs(chunk->k - m_currentChunk.k) > VIEW_SIZE)) {
                    if (chunk->state != CHUNK_NOT_LOADED) {
                        chunk->state = CHUNK_RECYCLE;

                        m_chunkToRecycle[m_chunkToRecycleCount++] = chunk;
                    }
                    if (chunk->vboIndex != -1) {
                        m_nextFreeBuffers.push_back(chunk->vboIndex);
                        Buffer* buffer = m_oglBuffers + chunk->vboIndex;
                        buffer->loaded = false;
                        chunk->vboIndex = -1;
                    }
                    int dx = camX - chunk->i*CHUNK_SIZE*CHUNK_SCALE;
                    int dy = camY - chunk->j*CHUNK_SIZE*CHUNK_SCALE;
                    int dz = camZ - chunk->k*CHUNK_SIZE*CHUNK_SCALE;
                    chunk->distanceFromCamera = dx * dx + dy * dy + dz * dz;
                } else { // Le chunk est dans la vue
                    if (chunk->state == CHUNK_RECYCLE) { // Les données sont déjà chargées
                        chunk->state = CHUNK_LOADED_FREE; // On restore juste l'état
                        m_chunkToGenerate[m_chunkToGenerateCount++] = chunk;
                    } else if(chunk->state == CHUNK_NOT_LOADED) { // Les données ne sont pas chargées
                        chunk->state = CHUNK_LOADING;
                        m_chunkToGenerate[m_chunkToGenerateCount++] = chunk;
                    } else {
                        m_chunkToDraw[m_chunkToDrawCount++] = chunk;
                    }
                    int dx = camX - chunk->i*CHUNK_SIZE*CHUNK_SCALE;
                    int dy = camY - chunk->j*CHUNK_SIZE*CHUNK_SCALE;
                    int dz = camZ - chunk->k*CHUNK_SIZE*CHUNK_SCALE;
                    chunk->distanceFromCamera = dx * dx + dy * dy + dz * dz;
                }
                ++ite;
            }
            for (int i = 0; i < m_chunkToGenerateCount; ++i) {
                if (m_chunkToGenerate[i]->vboIndex == -1) {
                    m_chunkToGenerate[i]->vboIndex = m_nextFreeBuffers.back();
                    m_nextFreeBuffers.pop_back();
                }
            }
            // Il ne reste plus assez de chunks vide -> on doit recycler les anciens
            if (m_chunkToGenerateCount > m_chunkDataLeft) {
                // On trie les anciens chunks pour ne supprimer les plus loins en premier
                std::sort(m_chunkToRecycle, m_chunkToRecycle + m_chunkToRecycleCount, [this](Chunk* i, Chunk* j)->bool{
                    return i->distanceFromCamera > j->distanceFromCamera;
                });
            }
            // On trie les nouveau chunks pour générer les plus près en premier
            std::sort(m_chunkToGenerate, m_chunkToGenerate + m_chunkToGenerateCount, [this](Chunk* i, Chunk* j)->bool{
                return i->distanceFromCamera < j->distanceFromCamera;
            });

            int recycle = 0;
            for (int i = 0; i < m_chunkToGenerateCount; ++i) {
                if (m_chunkDataLeft > 0) {
                    m_chunkToGenerate[i]->chunkBufferIndex = CHUNK_NUMBER - m_chunkDataLeft;
                    m_chunkDataLeft--;
                } else if (recycle < m_chunkToRecycleCount) {
                    m_chunkToRecycle[recycle]->state = CHUNK_NOT_LOADED;
                    m_chunkToGenerate[i]->chunkBufferIndex = m_chunkToRecycle[recycle]->chunkBufferIndex;
                    m_chunkToRecycle[recycle]->chunkBufferIndex = -1;

                    Coords c = {m_chunkToRecycle[recycle]->i, m_chunkToRecycle[recycle]->j, m_chunkToRecycle[recycle]->k};
                    m_ChunkMap[c] = nullptr;

                    recycle++;
                } else {
                    #ifdef QT_DEBUG
                    std::cout << "plus assez de buffer pour generer le mesh" << std::endl;
                    #endif
                }
            }
            for (int i = 0; i < m_chunkToGenerateCount; ++i) {
                m_chunkCommandsBuffer[i].chunk = m_chunkToGenerate[m_chunkToGenerateCount - i - 1];
                m_chunkCommandsBuffer[i].data = m_chunkBuffers + m_chunkToGenerate[m_chunkToGenerateCount - i - 1]->chunkBufferIndex * CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE;
                m_chunkCommandsBuffer[i].onlyLight = false;
            }
            m_chunkCommands.push(m_chunkCommandsBuffer, m_chunkToGenerateCount);

            for (int i = 0; i < m_chunkToGenerateCount; ++i) {
                m_chunkToDraw[m_chunkToDrawCount++] = m_chunkToGenerate[i];
            }
        } else {
            // On met à jour les lumières
            /*m_chunkToGenerateCount = 0;
            for (auto ite = m_ChunkMap.begin(); ite != m_ChunkMap.end(); ++ite) {
                if ((ite->second != nullptr) && ite->second->isLightDirty && (ite->second->state == CHUNK_LOADED_FREE)) {
                    ite->second->state = CHUNK_LOADING;
                    ite->second->isLightDirty = false;
                    m_chunkCommandsBuffer[m_chunkToGenerateCount].chunk = ite->second;
                    m_chunkCommandsBuffer[m_chunkToGenerateCount].data = m_chunkBuffers + ite->second->chunkBufferIndex * CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE;
                    m_chunkCommandsBuffer[m_chunkToGenerateCount].onlyLight = true;
                    m_chunkToGenerateCount++;
                }
            }
            m_chunkCommands.push(m_chunkCommandsBuffer, m_chunkToGenerateCount);*/
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

		for (int i = 0; i < m_chunkToDrawCount; ++i) {
			Chunk* chunk = m_chunkToDraw[i];
			Buffer* buffer = m_oglBuffers + chunk->vboIndex;

            if (buffer->loaded && buffer->draw && buffer->opaqueCount > 0) {
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
            if (buffer->loaded && buffer->draw && buffer->waterCount > 0) {
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
}

void ChunkManager::checkChunk(Coords pos) {

    Chunk* chunk = getChunkNoLock(pos);

	if (chunk == nullptr) {
		chunk = new Chunk();
		chunk->i = pos.i;
		chunk->j = pos.j;
		chunk->k = pos.k;

		m_ChunkMap[pos] = chunk;
	}
}

Voxel* ChunkManager::getBufferAdress(int index) {
	return index != -1 ? m_chunkBuffers + (index * CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE) : nullptr;
}

Chunk* ChunkManager::getChunk(Coords pos) {

	if(m_lastChunkId == pos && m_lastChunk != nullptr)
		return m_lastChunk;

	if (pos.j < 0 || pos.j >= WORLD_HEIGHT)
		return nullptr;

    m_mutexChunkManagerList.lock();
	auto it = m_ChunkMap.find(pos);
	auto end = m_ChunkMap.end();
    m_mutexChunkManagerList.unlock();
	if (it == end) {
		return nullptr;
	}

    if ((it->second != nullptr) && (it->second->state == CHUNK_LOADED_FREE)) {
        m_lastChunkId = pos;
        m_lastChunk = it->second;
        return m_lastChunk;
    }
    return nullptr;
}

Chunk* ChunkManager::getChunkNoLock(Coords pos) {

    if(m_lastChunkId == pos && m_lastChunk != nullptr)
        return m_lastChunk;

    if (pos.j < 0 || pos.j >= WORLD_HEIGHT)
        return nullptr;

    auto it = m_ChunkMap.find(pos);
    auto end = m_ChunkMap.end();
    if (it == end) {
        return nullptr;
    }

    if ((it->second != nullptr) && (it->second->state == CHUNK_LOADED_FREE)) {
        m_lastChunkId = pos;
        m_lastChunk = it->second;
        return m_lastChunk;
    }
    return nullptr;
}

Chunk* ChunkManager::getChunk(int i, int j, int k) {
	return getChunk({ i, j, k });
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

			Voxel& v = voxels[index];

			bool changed = false;
			res = v.type;

			if (light != NO_CHANGE){
				v._light = light;
			}

			v.type = newType;
			
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
	

	for (auto coords : modifiedChunks) {
		Chunk* chunk = getChunk(coords);
		if (chunk != nullptr) {
			chunk->isLightDirty = true;
		}
	}

	if (thisChunk != nullptr) {
		thisChunk->isLightDirty = true;
		thisChunk->isDirty = true;
	}
}
