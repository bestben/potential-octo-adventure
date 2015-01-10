#include "meshworker.h"

#include "vectorthreadsafe.h"
#include "../gamewindow.h"
#include "../chunkmanager.h"

#define VBO_MAX_SIZE CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE * 6 * 6
#define MAXIMUM_MESH_PER_FRAME 20

MeshGeneratorWorker::MeshGeneratorWorker(ChunkManager* cm) : m_commands{nullptr}, m_meshGenerator{cm},
                                m_readSemaphore{MAXIMUM_MESH_PER_FRAME}, m_needStop{false}, m_isWaiting{false} {
    m_verticesToUpload = new GLuint[VBO_MAX_SIZE * MAXIMUM_MESH_PER_FRAME];
    m_buffersToUpload = new Buffer*[MAXIMUM_MESH_PER_FRAME];
    m_toUpload = new std::atomic<bool>[MAXIMUM_MESH_PER_FRAME];
    for (int i = 0; i < MAXIMUM_MESH_PER_FRAME; ++i) {
        m_buffersToUpload[i] = nullptr;
        m_toUpload[i] = false;
    }

    m_canUpload = false;
}

MeshGeneratorWorker::~MeshGeneratorWorker() {
    delete[] m_toUpload;
    delete[] m_buffersToUpload;
    delete[] m_verticesToUpload;
}

void MeshGeneratorWorker::setBuffers(Buffer* buffers) {
    m_buffers = buffers;
}

void MeshGeneratorWorker::setCommandQueue(VectorThreadSafe<MeshWorkerCommand>* commands) {
    m_commands = commands;
}

void MeshGeneratorWorker::uploadBuffers(GameWindow* gl) {
    if (!m_canUpload) {
        return;
    }

    for (int i = 0; i < MAXIMUM_MESH_PER_FRAME; ++i) {
        if (m_toUpload[i]) {
            m_canUpload = false;
            gl->glBindBuffer(GL_ARRAY_BUFFER, m_buffersToUpload[i]->vbo);
            gl->glBufferData(GL_ARRAY_BUFFER, (m_buffersToUpload[i]->opaqueCount + m_buffersToUpload[i]->waterCount) * sizeof(GLuint),
                             m_verticesToUpload + VBO_MAX_SIZE * i, GL_STATIC_DRAW);
            gl->glBindBuffer(GL_ARRAY_BUFFER, 0);
            m_buffersToUpload[i]->loaded = true;
            m_toUpload[i] = false;
            m_readSemaphore.notify();
        }
    }
}


void MeshGeneratorWorker::requestStop() {
    m_needStop = true;
    m_readSemaphore.notifyAll(1000);
}

bool MeshGeneratorWorker::isWaiting() {
    return m_isWaiting;
}

void MeshGeneratorWorker::run() {
    if (m_commands != nullptr) {
        while (1) {
            if (m_needStop) {
                return;
            }
            m_isWaiting = true;
            std::cout << "wait upload" << std::endl;
            // Fonction bloquante
            m_readSemaphore.wait();
            std::cout << "end wait upload" << std::endl;
            m_isWaiting = false;
            if (m_needStop) {
                return;
            }
            int index = 0;
            for (index = 0; index < MAXIMUM_MESH_PER_FRAME;) {
                if (!m_toUpload[index]) {
                    break;
                }
                index++;
            }
            if (index == MAXIMUM_MESH_PER_FRAME) { // Aucun emplacement de libre
                continue;
            }
            m_isWaiting = true;
            // Fonction bloquante
            std::cout << "wait commande" << std::endl;
            MeshWorkerCommand command = m_commands->pop();
            std::cout << "end wait commande" << std::endl;
            m_isWaiting = false;
            if (m_needStop) {
                return;
            }

            Buffer* buffer = m_buffers + command.chunk->vboIndex;

            m_buffersToUpload[index] = buffer;

            m_meshGenerator.generate(command.data, {command.chunk->i, command.chunk->j, command.chunk->k},
                                     buffer, m_verticesToUpload + index * VBO_MAX_SIZE);
            m_toUpload[index] = true;
            m_canUpload = true;
        }
    }
}
