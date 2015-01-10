#pragma once

#include <QThread>
#include <atomic>

#include "../chunk.h"
#include "../biomes/ChunkGenerator.h"

template<class T> class VectorThreadSafe;
struct MeshWorkerCommand;
class GameWindow;
class ChunkManager;

struct ChunkWorkerCommand {
    Chunk* chunk;
    Voxel* data;
    bool onlyLight;
};

class ChunkWorker : public QThread {
public:
    ChunkWorker(ChunkManager* cm);
    ~ChunkWorker();

    void setCommandQueue(VectorThreadSafe<ChunkWorkerCommand>* commands);
    void setOutputCommandQueue(VectorThreadSafe<MeshWorkerCommand>* commands);

    void requestStop();
    void reset();
    bool isWaiting();

protected:
    void run();

private:
    VectorThreadSafe<ChunkWorkerCommand>* m_commands;
    VectorThreadSafe<MeshWorkerCommand>* m_output;

    ChunkGenerator m_chunkGenerator;

    ChunkManager* m_chunkManager;

    std::atomic<bool> m_needStop;
    std::atomic<bool> m_needReset;
    std::atomic<bool> m_isWaiting;
};
