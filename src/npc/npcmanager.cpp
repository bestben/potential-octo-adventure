#include "npcmanager.h"

#include "npc.h"

#include "creeper.h"

#define BASE_NPC_ALLOCATION 10

NpcManager::NpcManager() : m_npcs(BASE_NPC_ALLOCATION, nullptr) {

}

NpcManager::~NpcManager() {

}

void NpcManager::init(GameWindow* /*game*/) {
    /*float ratio = 360.0f / BASE_NPC_ALLOCATION;

    for (int i = 0; i < BASE_NPC_ALLOCATION; ++i) {
        float angle = (float)i * ratio;
        float x = cos(angle * 3.14f / 180.0f);
        float y = sin(angle * 3.14f / 180.0f);

        m_npcs[i] = new Creeper();
        m_npcs[i]->init(game);
        m_npcs[i]->setPosition(glm::vec3(x * 50.0f * CHUNK_SCALE, CHUNK_SCALE*CHUNK_SIZE*5.5f, y * 50.0f * CHUNK_SCALE));
    }*/
}

void NpcManager::destroy(GameWindow* game) {
    int size = m_npcs.size();
    for (int i = 0; i < size; ++i) {
        if (m_npcs[i] != nullptr) {
            m_npcs[i]->destroy(game);
            delete m_npcs[i];
        }
    }
}

void NpcManager::update(GameWindow* game, int dt) {
    int size = m_npcs.size();
    for (int i = 0; i < size; ++i) {
        if (m_npcs[i] != nullptr) {
            if (m_npcs[i]->canBeDestroyed()) {
                m_npcs[i]->destroy(game);
                delete m_npcs[i];
                m_npcs[i] = nullptr;
            } else {
                m_npcs[i]->update(game, dt);
            }
        }
    }
}

void NpcManager::draw(GameWindow* game) {
    int size = m_npcs.size();
    for (int i = 0; i < size; ++i) {
        if (m_npcs[i] != nullptr) {
            m_npcs[i]->draw(game);
        }
    }
}

int NpcManager::getFreeIndex() {
    int size = m_npcs.size();
    for (int i = 0; i < size; ++i) {
        if (m_npcs[i] == nullptr) {
            return i;
        }
    }
    m_npcs.push_back(nullptr);
    return size;
}
