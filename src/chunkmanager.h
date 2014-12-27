#pragma once

#include <QtGui/QOpenGLFunctions>
#include <atomic>
#include <vector>
#include <tuple>
#include <QThread>
#include <QLinkedList>

#include "chunk.h"
#include "meshgenerator.h"
#include "biomes/ChunkGenerator.h"

class QOpenGLVertexArrayObject;
class QOpenGLShaderProgram;
class QOpenGLTexture;
class GameWindow;

struct Buffer {
    QOpenGLVertexArrayObject* vao;
    GLuint vbo;
    unsigned int count;
    bool draw;
};

/**
 * @brief Classe gérant les chunks devant être chargés/déchargés/affichés.
 */
class ChunkManager : QThread {
public:
    ChunkManager();
    ~ChunkManager();

	void initialize(GameWindow* gl);
    void update(GameWindow* gl);
    void draw(GameWindow* gl);
	void checkChunk(Coords tuple);
	void requestChunks();
	Voxel* getBufferAdress(int index);
	void destroy(GameWindow* gl);

	Chunk& getChunk(Coords pos);
	Chunk& getChunk(int i, int j, int k);

protected:
    void run();

private:
    
    int seekFreeChunkData();
    int seekFreeBuffer();

    bool m_isInit;

    MeshGenerator m_meshGenerator;
    // Le shader affichant un chunk
    QOpenGLShaderProgram* m_program;
    // L'atlas de textures
    QOpenGLTexture* m_atlas;

    int m_posAttr;
    int m_matrixUniform;
    int m_chunkPosUniform;

    // Le tableau contenant tous les voxels des chunks
    Voxel* m_chunkBuffers;
    std::atomic<bool>* m_availableChunkData;
    std::atomic<bool>* m_inUseChunkData;

    // Le tableau des buffers opengl
    Buffer* m_oglBuffers;

    //std::map<std::tuple<int, int, int>, Chunk> m_ChunkMap;
	QHash<Coords, Chunk*> m_ChunkMap;

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

	QMutex m_mutexChunkManagerList;
	/*
	QLinkedList<Chunk*> m_toInvalidateChunkData;
	
	QLinkedList<Chunk*> m_toInvalidateBuffer;
	*/
	QLinkedList<Chunk*> m_toGenerateChunkData;
	QLinkedList<Chunk*> m_toGenerateBuffer;
	

	/*
    std::vector<std::tuple<int, int, int>> m_toInvalidateChunkData;
    std::vector<std::tuple<int, int, int>> m_toGenerateChunkData;

    std::vector<std::tuple<int, int, int>> m_toInvalidateBuffer;
    std::vector<std::tuple<int, int, int>> m_toGenerateBuffer;
	*/

    GLuint* m_tempVertexData;
    int m_vboToUpload;
    int m_countToUpload;

    std::atomic<bool> m_canGenerateMesh;
    std::atomic<bool> m_canUploadMesh;

	ChunkGenerator m_ChunkGenerator;
};
