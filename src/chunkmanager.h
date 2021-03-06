#pragma once

#include <QtGui/QOpenGLFunctions>
#include <atomic>
#include <QThread>
#include <QLinkedList>
#include <QTime>

#include "chunk.h"
#include "meshgenerator.h"
#include "biomes/ChunkGenerator.h"
#include "LightManager.h"
#include <QtCore/qvector.h>
#include <mutex>
#include <unordered_map>

class QOpenGLVertexArrayObject;
class QOpenGLShaderProgram;
class QOpenGLTexture;
class GameWindow;
class MeshGenerator;
class ChunkGenerator;


#define MAX_REMESH_PER_UPDATE 8
struct Buffer {
    QOpenGLVertexArrayObject* vao;
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
class ChunkManager : QThread {
public:
	ChunkManager(int worldSeed);
    ~ChunkManager();

	void initialize(GameWindow* gl);
    void update(GameWindow* gl);
	void draw(GameWindow* gl);
	void checkChunk(Coords tuple);
	
	void requestChunks();
	Voxel* getBufferAdress(int index);
	void destroy(GameWindow* gl);

	Chunk* getChunk(Coords pos);
	Chunk* getChunk(int i, int j, int k);


    Voxel getVoxel(int x, int y, int z, bool* loaded = nullptr);
	Voxel getVoxel(Coords c);
	
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
    
    Coords m_lastChunkId;
    std::atomic<Chunk*> m_lastChunk;

    int seekFreeChunkData();
    int seekFreeBuffer();

	int mWorldSeed;

    bool m_isInit;

    MeshGenerator* m_meshGenerator;
    // Le shader affichant un chunk
    QOpenGLShaderProgram* m_program;
    // Le shader affichant l'eau
    QOpenGLShaderProgram* m_waterProgram;
    // L'atlas de textures
    QOpenGLTexture* m_atlas;

    int m_posAttr;
    int m_matrixUniform;
    int m_chunkPosUniform;
    int m_lightMapUniform;

    int m_waterMatrixUniform;
    int m_waterChunkPosUniform;

    // Le tableau contenant tous les voxels des chunks
    Voxel* m_chunkBuffers;
    std::atomic<bool>* m_availableChunkData;
    std::atomic<bool>* m_inUseChunkData;

	uint16 m_chunkDataLeft;
	uint16 m_vboLeft;

    // Le tableau des buffers opengl
    Buffer* m_oglBuffers;
    Chunk** m_chunkToDraw;
    int m_chunkToDrawCount;

    std::unordered_map<Coords, Chunk*> m_ChunkMap;

    int m_currentChunkI;
    int m_currentChunkJ;
    int m_currentChunkK;

	Coords m_currentChunk;

    /////////////////////////
    /// Variables du second thread
    /////////////////////////

    std::atomic<bool> m_needRegen;
    std::atomic<bool> m_generationIsRunning;

    std::atomic<bool>* m_availableBuffer;

    std::mutex m_mutexChunkManagerList;
    std::mutex m_mutexGenerateQueue;

	QVector<Chunk*> m_toGenerateChunkData;
	QLinkedList<Chunk*> m_toGenerateBuffer;

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

    QTime m_animationTime;
};
