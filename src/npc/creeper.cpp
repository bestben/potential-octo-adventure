#include "creeper.h"

#include "../physic/physicmanager.h"
#include "../physic/body.h"
#include "../gamewindow.h"
#include "pathfinding.hpp"

#include <iostream>

#define GLM_FORCE_PURE
#include "glm/geometric.hpp"

Creeper::Creeper() : m_lengthOfSight{36}, m_pathRefreshRate{1000}, m_life{10}, m_canBeDestroyed{false} {

}

Creeper::~Creeper() {

}


void Creeper::init(GameWindow* gl) {
    int bodyID = gl->getPhysicManager().allocBody();
    m_body = gl->getPhysicManager().getBody(bodyID);

    m_body->jumpSpeed = 150.0f;
    m_body->position = glm::vec3(0.0f, CHUNK_SCALE*CHUNK_SIZE*5.5f, 0.0f );
    m_box.setSize(m_body->width, m_body->height);
    m_box.init(gl);

    m_pathfinding = new Pathfinding(gl->getChunkManager(), 100);

    /*m_refreshTimer.setInterval(m_pathRefreshRate);
    m_refreshTimer.connect(&m_refreshTimer, &Timer::timeout, [this]() {
        updatePath();
    });*/

    m_playerPosition = GetVoxelPosFromWorldPos(gl->getCamera().getFootPosition());
    //m_refreshTimer.start();
}

void Creeper::destroy(GameWindow* gl) {
    m_box.destroy(gl);

    delete m_pathfinding;
}

void Creeper::update(GameWindow* gl, int /*dt*/) {
    m_playerPosition = GetVoxelPosFromWorldPos(gl->getCamera().getFootPosition());

    Coords current = GetVoxelPosFromWorldPos(m_body->position);
    if ((std::abs(current.i - m_playerPosition.i) < m_lengthOfSight) &&
        (std::abs(current.j - m_playerPosition.j) < m_lengthOfSight) &&
        (std::abs(current.k - m_playerPosition.k) < m_lengthOfSight)) {
        m_box.setColor(1.0f, 0.0f, 0.0f);

        if (m_path.size() > 0) {
            Coords next = m_path.back();
            if (next == current) {
                m_path.pop_back();
                if (m_path.size() > 0) {
                    next = m_path.back();
                }
            }
            while ((m_path.size() > 0) && (next.i == current.i) && (next.k == current.k) && (next.j > current.j)) {
                m_path.pop_back();
                if (m_path.size() > 0) {
                    next = m_path.back();
                }
            }
            if (m_path.size() > 0) {
                glm::vec3 target = voxelToWorld(next);
                glm::vec3 direction = glm::normalize(target - m_body->position);
                glm::vec3 move = direction * 50.0f;
                m_body->force = move;
                if (next.j > current.j) {
                    m_body->jump = true;
                } else {
                    m_body->jump = false;
                }
            }
        }
    } else {
        m_box.setColor(0.0f, 0.0f, 1.0f);
        m_path.clear();
    }
}

void Creeper::draw(GameWindow* gl) {
    m_box.setPosition(m_body->position - glm::vec3(m_body->width / 2.0f, 0.0f, m_body->width / 2.0f));
    m_box.draw(gl);
}

void Creeper::updatePath() {
    if (m_body->onGround) {
        Coords current = GetVoxelPosFromWorldPos(m_body->position);
        if ((std::abs(current.i - m_playerPosition.i) < m_lengthOfSight) &&
            (std::abs(current.j - m_playerPosition.j) < m_lengthOfSight) &&
            (std::abs(current.k - m_playerPosition.k) < m_lengthOfSight)) {
            m_path = m_pathfinding->getPath(current, m_playerPosition);
        }
    }
}

bool Creeper::isDead() {
    return m_life > 0;
}

bool Creeper::canBeDestroyed() {
    return m_canBeDestroyed;
}

void Creeper::setPosition(const glm::vec3& pos) {
    if (m_body != nullptr) {
        m_body->position = pos;
    }
}
