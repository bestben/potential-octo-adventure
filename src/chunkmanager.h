#ifndef CHUNKMANAGER_H
#define CHUNKMANAGER_H

#include <QtGui/QOpenGLFunctions>
#include <atomic>
#include <vector>
#include <tuple>
#include <QThread>

#define CHUNK_NUMBER 700
#define VBO_NUMBER 700
#define CHUNK_SIZE 31
#define VIEW_SIZE 4
using Voxel = unsigned char;

class QOpenGLVertexArrayObject;
class GameWindow;

struct Chunk {
    // Les coordonées du chunk
    int i;
    int j;
    int k;

    bool visible;
    float distanceFromCamera;

    int chunkBufferIndex;
    int vboIndex;
};

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
    void destroy(GameWindow* gl);

    /**
     * @brief Renvoie le pointeur sur les données d'un chunk.
     * Les données sont bloquées tant que "unlockChunkData" n'est pas appelée.
     * @return nullptr si le chunk n'est pas disponible.
     */
    Voxel* lockChunkData(int i, int j, int k);

    void unlockChunkData(int i, int j, int k);

protected:
    void run();

private:
    Chunk& getChunk(int i, int j, int k);
    int seekFreeChunkData();
    int seekFreeBuffer();

    bool m_isInit;

    // Le tableau contenant tous les voxels des chunks
    Voxel* m_chunkBuffers;
    std::atomic<bool>* m_availableChunkData;
    std::atomic<bool>* m_inUseChunkData;

    // Le tableau des buffers opengl
    Buffer* m_oglBuffers;

    std::map<std::tuple<int, int, int>, Chunk> m_ChunkMap;

    int m_currentChunkI;
    int m_currentChunkJ;
    int m_currentChunkK;

    /////////////////////////
    /// Variables du second thread
    /////////////////////////

    std::atomic<bool> m_needRegen;
    std::atomic<bool> m_generationIsRunning;

    std::atomic<bool>* m_availableBuffer;

    std::vector<std::tuple<int, int, int>> m_toInvalidateChunkData;
    std::vector<std::tuple<int, int, int>> m_toGenerateChunkData;

    std::vector<std::tuple<int, int, int>> m_toInvalidateBuffer;
    std::vector<std::tuple<int, int, int>> m_toGenerateBuffer;

    GLuint* m_tempVertexData;
    int m_vboToUpload;
    int m_countToUpload;

    std::atomic<bool> m_canGenerateMesh;
    std::atomic<bool> m_canUploadMesh;
};

#endif // CHUNKMANAGER_H
