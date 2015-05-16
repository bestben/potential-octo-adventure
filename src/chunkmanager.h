#pragma once

#include <QtGui/QOpenGLFunctions>
#include "utilities/time.h"

#include <atomic>
#include <thread>
#include <mutex>
#include <unordered_map>
#include <memory>

#include "chunk.h"
#include "meshgenerator.h"
#include "biomes/ChunkGenerator.h"
#include "LightManager.h"
#include "utility.h"

class OpenGLVertexArrayObject;
class OpenglProgramShader;
class OpenGLTexture;
class GameWindow;
class MeshGenerator;
class ChunkGenerator;


#define MAX_REMESH_PER_UPDATE 4
struct Buffer {
    OpenGLVertexArrayObject* vao;
    GLuint vbo;
    unsigned int opaqueCount;
    unsigned int waterCount;
	unsigned int toUpOpaqueCount;
	unsigned int toUpWaterCount;
    bool draw;
	GLuint* toUpData;
};

/**
 * @brief Classe gérant les chunks devant être chargés/déchargés/affichés.
 */
class ChunkManager {
public:
	ChunkManager(int worldSeed);
    ~ChunkManager();

	void initialize(GameWindow* gl);
    void update(GameWindow* gl);
	void draw(GameWindow* gl);
	
	Voxel* getBufferAdress(int index);
	void destroy(GameWindow* gl);

	Chunk* getChunk(Coords pos);
	Chunk* getChunk(int i, int j, int k);

    MI_FORCE_INLINE Voxel getVoxel(int x, int y, int z, bool* loaded = nullptr);
    MI_FORCE_INLINE Voxel getVoxel(Coords c);
	
	LightManager& getLightManager();
	VoxelType placeVoxel(Coords pos, VoxelType type);
	void removeVoxel(Coords pos);
	/**
     * @brief Modifie un voxel et lance la reconstruction du mesh.
     * @param x Coordonnée du voxel.
     * @param y Coordonnée du voxel.
     * @param z Coordonnée du voxel.
     * @return L'ancien type du voxel.
     */
	VoxelType setVoxel(int x, int y, int z, VoxelType newType, uint light = NO_CHANGE);
	VoxelType setVoxel(Coords c, VoxelType newType, uint light = NO_CHANGE);


protected:
    void run();

private:
    int getArrayIndex(Coords chunkPos, Coords center);
    int getArrayIndex(int i, int j, int k, Coords center);
    Coords getChunkCoords(int index, Coords center);

	std::thread		m_thread;

	int mWorldSeed;

    bool m_isInit;

    MeshGenerator* m_meshGenerator;
    // Le shader affichant un chunk
    std::unique_ptr<OpenglProgramShader> m_program;
    // Le shader affichant l'eau
    std::unique_ptr<OpenglProgramShader> m_waterProgram;
    // L'atlas de textures
    std::unique_ptr<OpenGLTexture> m_atlas;

    int m_posAttr;
    int m_matrixUniform;
    int m_chunkPosUniform;
    int m_lightMapUniform;

    int m_waterMatrixUniform;
    int m_waterChunkPosUniform;
    int m_waterTimerUniform;

    // Le tableau contenant tous les voxels des chunks
    Voxel* m_chunkBuffers;

    Chunk* m_chunks;
    Chunk** m_chunksMapping[2];
    Coords m_chunksCenter[2];
    std::atomic<int> m_mapIndex;
    std::vector<Chunk*> m_nextFreeBuffer;

    // Le tableau des buffers opengl
    Buffer* m_oglBuffers;
    Chunk** m_chunkToDraw;
    int m_chunkToDrawCount;

	Coords m_currentChunk;

    /////////////////////////
    /// Variables du second thread
    /////////////////////////

    std::atomic<bool> m_needRegen;

    std::mutex m_mutexGenerateQueue;

    std::vector<Chunk*> m_toGenerateChunkData;

    GLuint* m_tempVertexData;
    Buffer m_tempBufferToUpload;
    int m_vboToUpload;
    int m_countToUpload;
	Chunk* m_chunkToUpload;

    std::atomic<bool> m_canGenerateMesh;
    std::atomic<bool> m_canUploadMesh;

	ChunkGenerator* m_ChunkGenerator;
	LightManager* m_LightManager;

	bool m_FirstUpdate;

    Time m_animationTime;

    std::atomic<Chunk*> m_lastChunk;
};
