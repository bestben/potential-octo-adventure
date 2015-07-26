#include "chunkmanager.h"
#include "gamewindow.h"

#include <iostream>
#include <algorithm>

#include "utilities/openglvertexarrayobject.h"
#include "utilities/openglprogramshader.h"
#include "utilities/opengltexture.h"

#define GLM_FORCE_PURE
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

MI_FORCE_INLINE bool chunkSortFunction(const ChunkSortData& i, const ChunkSortData& j) {
	return i.iDistance < j.iDistance;
}

ChunkManager::ChunkManager(int worldSeed) 
	: m_isInit{ false }
	, m_chunkBuffers{ nullptr }
	, m_oglBuffers{ nullptr }
	, m_FirstUpdate{ true }
	, m_oIOManager{ worldSeed } {

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
	m_chunkToSort = new ChunkSortData[VBO_NUMBER];

	m_needRegen = true;

	m_tempVertexData = new GLuint[CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE * 6 * 6];

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
	m_lastChunk = nullptr;
}

ChunkManager::~ChunkManager() {
    
    for (int i = 0; i < CHUNK_NUMBER; ++i) {
        Chunk* chunk = m_chunks + i;
        if (chunk->generated && chunk->differsFromDisk) {
            Voxel* voxel_data = chunk->data;
			m_oIOManager.saveChunk(Coords{ chunk->i, chunk->j, chunk->k }, voxel_data);
		}
	}

	delete[] m_chunkBuffers;
	delete[] m_tempVertexData;
	delete[] m_chunkToDraw;
	delete[] m_chunkToSort;

	delete m_ChunkGenerator;
    delete m_LightManager;
    delete m_meshGenerator;

    delete[] m_chunks;
    delete[]m_chunksMapping[0];
    delete[]m_chunksMapping[1];
}

void ChunkManager::initialize(GameWindow* /*gl*/) {
    m_program = std::make_unique<OpenglProgramShader>();
    m_program->addShaderFromSourceFile(OpenGLShaderType::Vertex, "shaders/render.vs");
    m_program->addShaderFromSourceFile(OpenGLShaderType::Fragment, "shaders/render.ps");
	if (!m_program->link()) {
		// TODO(antoine): Remove Force crash
		MI_ASSERT( false );
		abort();
	}
	m_posAttr = m_program->attributeLocation("position");
	m_matrixUniform = m_program->uniformLocation("viewProj");
	m_chunkPosUniform = m_program->uniformLocation("chunkPosition");

    glm::vec3 skyColor(0.53f, 0.807f, 0.92f);

	m_program->bind();
	m_program->setUniformValue("atlas", 0);
	m_program->setUniformValue("tileCount", 16);
	m_program->setUniformValue("tileSize", 16);
	m_program->setUniformValue("fogDistance", (float)(VIEW_SIZE*CHUNK_SIZE*CHUNK_SCALE)*3.0f);

    glUniform3fv(m_program->uniformLocation("fogColor"), 1, glm::value_ptr(skyColor));
    //m_program->setUniformValue("fogColor", skyColor);
	m_program->release();

    m_waterProgram = std::make_unique<OpenglProgramShader>();
    m_waterProgram->addShaderFromSourceFile(OpenGLShaderType::Vertex, "shaders/water.vs");
    m_waterProgram->addShaderFromSourceFile(OpenGLShaderType::Fragment, "shaders/water.ps");
	if (!m_waterProgram->link()) {
		// TODO(antoine): Remove Force crash
		MI_ASSERT(false);
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
    glUniform3fv(m_waterProgram->uniformLocation("fogColor"), 1, glm::value_ptr(skyColor));
    //m_waterProgram->setUniformValue("fogColor", skyColor);
	m_waterProgram->release();

    m_atlas = std::make_unique<OpenGLTexture>("textures/atlas.png");
	m_atlas->setMagnificationFilter(OpenGLTexture::Nearest);

	GLuint vbos[VBO_NUMBER];
	glGenBuffers(VBO_NUMBER, vbos);


	m_oglBuffers = new Buffer[VBO_NUMBER];
	for (int i = 0; i < VBO_NUMBER; ++i) {
		Buffer* buffer = m_oglBuffers + i;
		buffer->opaqueCount = 0;
		buffer->waterCount = 0;
		buffer->draw = false;

		buffer->vbo = vbos[i];
		buffer->vao = new OpenGLVertexArrayObject();
		buffer->vao->create();
		buffer->vao->bind();

		glBindBuffer(GL_ARRAY_BUFFER, buffer->vbo);
		glEnableVertexAttribArray(m_posAttr);
		glVertexAttribIPointer(m_posAttr, 1, GL_UNSIGNED_INT, 0, 0);
		buffer->vao->release();

		buffer->toUpData = nullptr; 

	}

	m_isInit = true;
	
	m_thread = std::thread( &ChunkManager::run, this );
}

void ChunkManager::destroy(GameWindow* /*gl*/) {
	
	m_needRegen = false;
	m_thread.join();

	m_isInit = false;

	// On nettoie les ressources opengl
	for (int i = 0; i < VBO_NUMBER; ++i) {
		Buffer* buffer = m_oglBuffers + i;

		buffer->vao->destroy();
		delete buffer->vao;
		glDeleteBuffers(1, &buffer->vbo);


		if (buffer->toUpData)
			delete buffer->toUpData;
	}

	// On supprime les tableaux
	delete[] m_oglBuffers;
	m_oglBuffers = nullptr;
	// On supprime l'atlas et le shader
	m_atlas = nullptr;
    m_program = nullptr;
    m_waterProgram = nullptr;
}

void ChunkManager::update(GameWindow* gl) {
	m_chunkToDrawCount = 0;
	Camera& oCamera = gl->getCamera();
	if (m_isInit) {

		glm::vec3 camPos = gl->getCamera().getPosition();

		Coords chunkHere = GetChunkPosFromVoxelPos(GetVoxelPosFromWorldPos(camPos));

        float camX = gl->getCamera().getPosition().x;
        float camY = gl->getCamera().getPosition().y;
        float camZ = gl->getCamera().getPosition().z;

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
                    for( int j = 0; j < (int)m_toGenerateChunkData.size(); ++j ) {
                        if( m_toGenerateChunkData[j] == chunk ) {
                            m_toGenerateChunkData[j] = nullptr;
                            break;
                        }
                    }
                    chunk->inQueue = false;
                    m_mutexGenerateQueue.unlock();
					chunk->visible = false;
					chunk->ready = false;
					chunk->isDirty = true;
					chunk->isLightDirty = true;
					chunk->onlyAir = true;
					chunk->differsFromDisk = false;
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
		int iChunkToSort = 0;
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
                    glBindBuffer(GL_ARRAY_BUFFER, buffer->vbo);
                    // On alloue un buffer plus grand si besoin
					if (totalCount > (buffer->opaqueCount + buffer->waterCount)) {
						glBufferData(GL_ARRAY_BUFFER, totalCount * sizeof(GLuint), m_tempVertexData, GL_STATIC_DRAW);
					} else {
						void *dataMap = glMapBufferRange(GL_ARRAY_BUFFER, 0, totalCount*sizeof(GLuint), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
						if (dataMap != nullptr) 
							memcpy(dataMap, m_tempVertexData, totalCount*sizeof(GLuint));
						glUnmapBuffer(GL_ARRAY_BUFFER);
					}
					chunk->ready = true;
					chunk->visible = true;
					buffer->opaqueCount = buffer->toUpOpaqueCount;
					buffer->waterCount = buffer->toUpWaterCount;
					buffer->draw = true;
					remesh = false;
                    glBindBuffer(GL_ARRAY_BUFFER, 0);

                } else {
                    buffer->draw = true;
                    chunk->ready = true;
                    chunk->visible = true;
                }
            }

            if (chunk->ready && chunk->generated && chunk->visible && !chunk->onlyAir) {
				//bool bInFrustum = oCamera.boxInFrustum(chunk->i*CHUNK_SIZE*CHUNK_SCALE, chunk->j*CHUNK_SIZE*CHUNK_SCALE, chunk->k*CHUNK_SIZE*CHUNK_SCALE, CHUNK_SIZE*CHUNK_SCALE);
				bool bInFrustum = oCamera.sphereInFrustum(
					glm::vec3(chunk->i*CHUNK_SIZE*CHUNK_SCALE + CHUNK_SIZE*CHUNK_SCALE / 2.0f,
							chunk->j*CHUNK_SIZE*CHUNK_SCALE + CHUNK_SIZE*CHUNK_SCALE / 2.0f,
							chunk->k*CHUNK_SIZE*CHUNK_SCALE + CHUNK_SIZE*CHUNK_SCALE / 2.0f),
					CHUNK_SIZE*CHUNK_SCALE);
				if (bInFrustum)
				{
					int dx = (int)camX - chunk->i*CHUNK_SIZE*CHUNK_SCALE;
					int dy = (int)camY - chunk->j*CHUNK_SIZE*CHUNK_SCALE;
					int dz = (int)camZ - chunk->k*CHUNK_SIZE*CHUNK_SCALE;
					chunk->distanceFromCamera = dx * dx + dy * dy + dz * dz;

					m_chunkToSort[iChunkToSort].iDistance = chunk->distanceFromCamera;
					m_chunkToSort[iChunkToSort].iChunkIndex = i;
					++iChunkToSort;
				}
            }
        }

		std::sort(m_chunkToSort, m_chunkToSort + iChunkToSort, [](const ChunkSortData& i, const ChunkSortData& j)->bool {
			return i.iDistance < j.iDistance;
		});

		for (int i = 0; i < iChunkToSort; ++i) {
			m_chunkToDraw[m_chunkToDrawCount++] = m_chunks + m_chunkToSort[i].iChunkIndex;
		}
		
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

		glm::mat4x4 mat = gl->getCamera().getViewProjMatrix();
        glm::mat4x4 scale = glm::scale( glm::mat4x4(), glm::vec3((float)CHUNK_SCALE, (float)CHUNK_SCALE, (float)CHUNK_SCALE) );
        m_program->setUniformValue(m_matrixUniform, mat * scale);
		m_atlas->bind(0);

		for (int i = m_chunkToDrawCount - 1; i >= 0; --i) {
			Chunk* chunk = m_chunkToDraw[i];
			Buffer* buffer = m_oglBuffers + chunk->vboIndex;

            if (chunk->generated && buffer->draw && buffer->opaqueCount > 0) {
				buffer->vao->bind();
                glUniform3fv(m_chunkPosUniform, 1, glm::value_ptr(glm::vec3((float)(chunk->i * CHUNK_SIZE),
                                                                                (float)(chunk->j * CHUNK_SIZE),
                                                                                (float)(chunk->k * CHUNK_SIZE))));
				glDrawArrays(GL_TRIANGLES, 0, buffer->opaqueCount);
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
                glUniform3fv(m_waterChunkPosUniform, 1, glm::value_ptr(glm::vec3((float)(chunk->i * CHUNK_SIZE),
                                                                                    (float)(chunk->j * CHUNK_SIZE),
                                                                                    (float)(chunk->k * CHUNK_SIZE))));
				glDrawArrays(GL_TRIANGLES, buffer->opaqueCount, buffer->waterCount);
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
                if( c1 == nullptr )
                    return false;
                else if ( c2 == nullptr )
                    return true;
				int a = (c1->i - here.i)*(c1->i - here.i) + (c1->k - here.k)*(c1->k - here.k);
				int b = (c2->i - here.i)*(c2->i - here.i) + (c2->k - here.k)*(c2->k - here.k);
				if (a == b) {
                    return c1->j < c2->j;
				}
                return a > b;
            });

			
            Chunk* newChunk = nullptr;
            while( (newChunk == nullptr) && (m_toGenerateChunkData.size() > 0) ) {
                newChunk = m_toGenerateChunkData.back();
                m_toGenerateChunkData.pop_back();
            }
           if( newChunk == nullptr )
               continue;
			m_mutexGenerateQueue.unlock();

            if (!newChunk->generated) {
                // Générer le nouveau chunk et le prendre du DD si déja présent
                Voxel* data = newChunk->data;
                if (data != nullptr){

                    bool skipGeneration = false;
                    if (m_oIOManager.chunkExist(Coords{ newChunk->i, newChunk->j, newChunk->k })) {
						m_oIOManager.loadChunk(Coords{ newChunk->i, newChunk->j, newChunk->k }, data);
                        newChunk->differsFromDisk = !skipGeneration;
                    }

                    std::set<Coords> modifiedChunks;
                    if(!skipGeneration) {
						newChunk->onlyAir = m_ChunkGenerator->generateChunk(data, newChunk->i, newChunk->j, newChunk->k, modifiedChunks);
						m_oIOManager.saveChunk(Coords{ newChunk->i, newChunk->j, newChunk->k }, data);
                        newChunk->differsFromDisk = false;
                    }

                    m_LightManager->updateLighting(newChunk);
                    modifiedChunks.erase(Coords{ newChunk->i, newChunk->j, newChunk->k });
                    for (auto pos : modifiedChunks) {
                        Chunk* c = getChunk(pos);
                        if (c != nullptr){
                            if (c->generated) {
                                m_LightManager->updateLighting(c);
                                Voxel* voxel_data = c->data;
								m_oIOManager.saveChunk(Coords{ c->i, c->j, c->k }, voxel_data);
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

VoxelType ChunkManager::setVoxel(Coords c, VoxelType newType, uint8 light) {
	return setVoxel(c.i, c.j, c.k, newType, light);
}

VoxelType ChunkManager::setVoxel(int x, int y, int z, VoxelType newType, uint8 light) {
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
	
    std::set<Coords> modifiedChunks;

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

    std::set<Coords> modifiedChunks;
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
