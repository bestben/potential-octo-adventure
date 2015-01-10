#include "chunkworker.h"

#include "vectorthreadsafe.h"

#include "meshworker.h"
#include "../gamewindow.h"
#include <iostream>

ChunkWorker::ChunkWorker(ChunkManager* cm) : m_commands{nullptr}, m_chunkManager{cm}, m_needStop{false},
                                            m_needReset{false}, m_isWaiting{false} {

}

ChunkWorker::~ChunkWorker() {

}

void ChunkWorker::setCommandQueue(VectorThreadSafe<ChunkWorkerCommand>* commands) {
    m_commands = commands;
}

void ChunkWorker::setOutputCommandQueue(VectorThreadSafe<MeshWorkerCommand>* commands) {
    m_output = commands;
}

void ChunkWorker::requestStop() {
    m_needStop = true;
}

void ChunkWorker::reset() {
    m_needReset = true;
}

bool ChunkWorker::isWaiting() {
    return m_isWaiting;
}

void ChunkWorker::run() {
    if (m_commands != nullptr) {
        while (1) {
            if (m_needStop) {
                return;
            }
            m_isWaiting = true;
            // function bloquante
            ChunkWorkerCommand command = m_commands->pop();
            m_isWaiting = false;
            if (m_needStop) {
                return;
            }
            if (m_needReset) {
                m_needReset = false;
                continue;
            }

            if (command.chunk->state == CHUNK_LOADING) {
                if (!command.onlyLight) {
                    m_chunkGenerator.generateChunk(command.data, {command.chunk->i, command.chunk->j, command.chunk->k});
                }
                if (m_needReset) {
                    m_needReset = false;
                    continue;
                }
                command.chunk->state = CHUNK_LOADED_FREE;
                m_chunkManager->getLightManager().updateLighting(command.chunk);
                command.chunk->isLightDirty = false;
            }
            if (m_needReset) {
                m_needReset = false;
                continue;
            }

            MeshWorkerCommand cmd;
            cmd.chunk = command.chunk;
            cmd.data = command.data;
            m_output->push(cmd);
        }
    }
}
