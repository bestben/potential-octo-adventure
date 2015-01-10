#pragma once

#include <QtGui/QOpenGLFunctions>
#include <atomic>
#include <QThread>
#include <QLinkedList>
#include <QTime>
#include <unordered_map>
#include <mutex>

#include "chunk.h"
#include "meshgenerator.h"
#include "biomes/ChunkGenerator.h"
#include "LightManager.h"
#include <QtCore/qvector.h>

#include "chunkmanager/chunkworker.h"
#include "chunkmanager/vectorthreadsafe.h"
#include "chunkmanager/meshworker.h"

class QOpenGLVertexArrayObject;
class QOpenGLShaderProgram;
class QOpenGLTexture;
class GameWindow;
class MeshGenerator;

struct Buffer {
    QOpenGLVertexArrayObject* vao;
    GLuint vbo;
    unsigned int opaqueCount;
    unsigned int waterCount;
    bool draw;
    std::atomic<bool> loaded;
};

/**
 * @brief Classe gérant les chunks devant être chargés/déchargés/affichés.
 */
class ChunkManager {
public:
    ChunkManager();
    ~ChunkManager();

	void initialize(GameWindow* gl);
    void update(GameWindow* gl);
	void draw(GameWindow* gl);
	
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

private:
    void checkChunk(Coords tuple);
    Chunk* getChunkNoLock(Coords pos);

    bool m_isInit;

    // Le dernier chunk utilisé (optimisation pour éviter de rechercher le même à chaque fois)
    Coords m_lastChunkId;
    Chunk* m_lastChunk;

    // La position actuelle du joueur
    Coords m_currentChunk;

    // Le thread générant la création des chunks
    ChunkWorker m_chunkWorker;
    // Le thread générant la transformation des chunks
    MeshGeneratorWorker m_meshWorker;
    VectorThreadSafe<ChunkWorkerCommand> m_chunkCommands;
    VectorThreadSafe<MeshWorkerCommand> m_meshCommands;

    LightManager* m_LightManager;

    // Le tableau contenant tous les voxels des chunks
    Voxel* m_chunkBuffers;
    uint16 m_chunkDataLeft;

    std::unordered_map<Coords, Chunk*> m_ChunkMap;

    // Le tableau des buffers opengl
    Buffer* m_oglBuffers;
    std::vector<int> m_nextFreeBuffers;
    // Les chunk à dessiner
    Chunk** m_chunkToDraw;
    int m_chunkToDrawCount;


    ////////////////////////////////////////////////////////////////////
    /// Les tableaux stockants des données temporaires (évite les new/delete)
    ////////////////////////////////////////////////////////////////////

    // Les chunks à recycler (reset à chaque frame)
    Chunk** m_chunkToRecycle;
    int m_chunkToRecycleCount;
    // Les chunks à générer (reset à chaque frame)
    Chunk** m_chunkToGenerate;
    int m_chunkToGenerateCount;

    ChunkWorkerCommand* m_chunkCommandsBuffer;

    //////////////////////////////////
    /// Les variables de shaders
    //////////////////////////////////

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

    std::mutex m_mutexChunkManagerList;

	bool m_FirstUpdate;

    // Le timer servant à animer l'eau
    QTime m_animationTime;
};
