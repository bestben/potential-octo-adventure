#include "physicmanager.h"

#include <iostream>

#include "../gamewindow.h"

#include "body.h"

PhysicManager::PhysicManager() : m_freeBodies(BODY_COUNT, true), m_hasGravity{false} {
    m_bodies = new Body[BODY_COUNT];
    for (int i = 0; i < BODY_COUNT; ++i) {
        m_bodies[i].jump = false;
        m_bodies[i].height = 6;
        m_bodies[i].width = 1;
        m_bodies[i].mass = 1;
    }
}

PhysicManager::~PhysicManager() {
    delete[] m_bodies;
}

void PhysicManager::update(GameWindow* gl, int dt) {
    float delta = (float)dt / 1000.0;

    QVector3D g(0.0, -981, 0.0);

    for (int i = 0; i < BODY_COUNT; ++i) {
        if (!m_freeBodies[i]) {
            Body* body = m_bodies + i;
            QVector3D lastAcceleration = body->acceleration;

            QVector3D deltaPos = body->velocity * delta + (0.5 * lastAcceleration * delta * delta);
            QVector3D newPosition = body->position;
            QVector3D newAcceleration;
            QVector3D avgAcceleration;

            QVector3D force = body->force;
            if (force.x() == 0) {
                lastAcceleration.setX(0.0f);
            }
            if (force.z() == 0) {
                lastAcceleration.setZ(0.0f);
            }
            if (m_hasGravity) {
                force.setY(0.0f);
                force += g;
                if (body->jump) {
                    force -= 10 * g;
                }
                newAcceleration = force / body->mass;
                //avgAcceleration = (lastAcceleration + newAcceleration) * 0.5;
                avgAcceleration = newAcceleration;
                body->velocity += avgAcceleration * delta;

                // Test collision
                bool colliding = collide(gl, body, newPosition, QVector3D(0.0f, deltaPos.y(), 0.0f));
                if (colliding) {
                    avgAcceleration.setY(0.0f);
                    body->velocity.setY(0.0f);
                }
                colliding = collide(gl, body, newPosition, QVector3D(deltaPos.x(), 0.0f, 0.0f));
                if (colliding) {
                    avgAcceleration.setX(0.0f);
                    body->velocity.setX(0.0f);
                }

                colliding = collide(gl, body, newPosition, QVector3D(0.0f, 0.0f, deltaPos.z()));
                if (colliding) {
                    avgAcceleration.setZ(0.0f);
                    body->velocity.setZ(0.0f);
                }
            } else {
                avgAcceleration = QVector3D(0.0f, 0.0f, 0.0f);
                body->velocity = force*10.0;
                newPosition += deltaPos / 10;
            }

            body->acceleration = avgAcceleration;
            body->jump = false;

            body->position = newPosition;
        }
    }
}

bool PhysicManager::collide(GameWindow* gl, Body* body, QVector3D& position, const QVector3D& delta) {
    const float PADDING = 0.000001;
    ChunkManager& chunkManager = gl->getChunkManager();
    QVector3D corners[] = {
        QVector3D(body->width + PADDING, - PADDING, body->width + PADDING),
        QVector3D(body->width + PADDING, - PADDING, -body->width - PADDING),
        QVector3D(-body->width - PADDING, - PADDING, body->width + PADDING),
        QVector3D(-body->width - PADDING, - PADDING, -body->width - PADDING),

        QVector3D(body->width, body->height, body->width),
        QVector3D(body->width, body->height, -body->width),
        QVector3D(-body->width, body->height, body->width),
        QVector3D(-body->width, body->height, -body->width)
    };

    bool isColliding = false;

    for (int i = 0; i < 4; ++i) {
        QVector3D oldVoxel = (corners[i] + position) / (CHUNK_SCALE);
        oldVoxel = QVector3D(floor(oldVoxel.x()), floor(oldVoxel.y()), floor(oldVoxel.z()));
        QVector3D newVoxel = (corners[i] + position + delta) / (CHUNK_SCALE);
        newVoxel = QVector3D(floor(newVoxel.x()), floor(newVoxel.y()), floor(newVoxel.z()));
        if ((oldVoxel != newVoxel) && (getVoxelType(chunkManager.getVoxel(newVoxel.x(), newVoxel.y(), newVoxel.z())) != VoxelType::AIR)) {
            std::cout << "old : " << oldVoxel.x() << " " << oldVoxel.y() << " " << oldVoxel.z() << std::endl;
            std::cout << "new : " << newVoxel.x() << " " << newVoxel.y() << " " << newVoxel.z() << std::endl;
            QVector3D direction = oldVoxel - newVoxel;
            QVector3D size = direction * QVector3D(body->width, corners[i].y(), body->width);
            std::cout << "size : " << size.x() << " " << size.y() << " " << size.z() << std::endl;
            direction = (direction * (CHUNK_SCALE + PADDING) / 2) + size;
            std::cout << "direction : " << direction.x() << " " << direction.y() << " " << direction.z() << std::endl;

            QVector3D correctedPos = (newVoxel + QVector3D(0.5f, 0.5f, 0.5f)) * CHUNK_SCALE + direction;
            std::cout << "position0 : " << position.x() << " " << position.y() << " " << position.z() << std::endl;
            if (direction.x() != 0.0f) {
                position.setX(correctedPos.x());
            } else if (direction.y() != 0.0f) {
                position.setY(correctedPos.y());
            } else if (direction.z() != 0.0f) {
                position.setZ(correctedPos.z());
            }
            std::cout << "position1 : " << position.x() << " " << position.y() << " " << position.z() << std::endl;

            isColliding = true;
        }
    }
    if (!isColliding) {
        //std::cout << "No collide : " << delta.x() << " " << delta.y() << " " << delta.z() << std::endl;
        position += delta;
    }

    return isColliding;
}

int PhysicManager::allocBody() {
    for (int i = 0; i < BODY_COUNT; ++i) {
        if (m_freeBodies[i] == true) {
            m_freeBodies[i] = false;
            return i;
        }
    }
    return -1;
}

void PhysicManager::freeBody(int bodyIndex) {
    if ((bodyIndex < 0) || (bodyIndex >= BODY_COUNT)) {
        return;
    }
    m_freeBodies[bodyIndex] = true;
}

Body* PhysicManager::getBody(int bodyIndex) {
    if ((bodyIndex < 0) || (bodyIndex >= BODY_COUNT)) {
        return nullptr;
    }
    return m_bodies + bodyIndex;
}

void PhysicManager::setGravity(bool active) {
    m_hasGravity = active;
}

