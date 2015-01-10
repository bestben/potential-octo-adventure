#pragma once

#include <QThread>
#include <atomic>
#include <mutex>

#include "../meshgenerator.h"
#include "../chunk.h"
#include "semaphore.h"

template<class T> class VectorThreadSafe;
struct Buffer;
class GameWindow;
class ChunkManager;

struct MeshWorkerCommand {
    Chunk* chunk;
    Voxel* data;
};

class MeshGeneratorWorker : public QThread {
public:
    MeshGeneratorWorker(ChunkManager* cm);
    ~MeshGeneratorWorker();

    void setBuffers(Buffer* buffers);
    void setCommandQueue(VectorThreadSafe<MeshWorkerCommand>* commands);

    /**
     * @brief Copie les buffers en mémoire.
     * !!!! Doit etre utilisé dans le thread de rendu !!!!!
     */
    void uploadBuffers(GameWindow* gl);

    void requestStop();
    bool isWaiting();

protected:
    void run();

private:
    VectorThreadSafe<MeshWorkerCommand>* m_commands;

    // Les buffers opengl du chunkManager
    Buffer* m_buffers;

    MeshGenerator m_meshGenerator;

    GLuint* m_verticesToUpload;
    Buffer** m_buffersToUpload;
    std::atomic<bool>* m_toUpload;

    Semaphore m_readSemaphore;
    std::mutex m_writeMutex;
    std::atomic<bool> m_canUpload;


    std::atomic<bool> m_needStop;
    std::atomic<bool> m_isWaiting;
};
