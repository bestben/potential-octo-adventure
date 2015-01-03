#include "npc.h"

#include "../physic/physicmanager.h"
#include "../physic/body.h"
#include "../gamewindow.h"
#include "pathfinding.hpp"

Npc::Npc() {

}

Npc::~Npc() {

}


void Npc::init(GameWindow* gl) {
    int bodyID = gl->getPhysicManager().allocBody();
    m_body = gl->getPhysicManager().getBody(bodyID);

    m_body->position = QVector3D(0.0f, CHUNK_SCALE*CHUNK_SIZE*5.5f, 0.0f );

    m_box.init(gl);

    m_pathfinding = new Pathfinding(gl->getChunkManager());
}

void Npc::destroy(GameWindow* gl) {
    m_box.destroy(gl);

    delete m_pathfinding;
}

void Npc::update(GameWindow* gl, int dt) {
    if (m_body->onGround) {
        Coords current = GetVoxelPosFromWorldPos(m_body->position);
        if (m_path.size() == 0) {
            m_path = m_pathfinding->getPath(current, GetVoxelPosFromWorldPos(gl->getCamera().getPosition()));
        }
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
                QVector3D target = voxelToWorld(next);
                QVector3D direction = target - m_body->position;
                direction.normalize();
                QVector3D move = direction * 50.0f;
                m_body->force = move;
                if (next.j > current.j) {
                    m_body->jump = true;
                }
            }
        }
    }
}

void Npc::draw(GameWindow* gl) {
    m_box.setPosition(m_body->position);
    m_box.draw(gl);
}
