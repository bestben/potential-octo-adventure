#include "physicmanager.h"

#include <iostream>

#include "../gamewindow.h"

#include "body.h"

#define JUMP_SPEED 300

PhysicManager::PhysicManager() : m_freeBodies(BODY_COUNT, true), m_hasGravity{false} {
    m_bodies = new Body[BODY_COUNT];
    for (int i = 0; i < BODY_COUNT; ++i) {
        m_bodies[i].jump = false;
        m_bodies[i].height = 12;
        m_bodies[i].width = 1;
        m_bodies[i].mass = 1;
        m_bodies[i].onGround = false;
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
                // Test collision
                bool colliding = collide(gl, body, newPosition, QVector3D(0.0f, deltaPos.y(), 0.0f));
                if (colliding) {
                    avgAcceleration.setY(0.0f);
                    if (body->velocity.y() < 0.0f) {
                        body->velocity.setY(0.0f);
                    }
                    m_bodies[i].onGround = true;
                } else {
                    m_bodies[i].onGround = false;
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

                // Gestion physique
                force.setY(0.0f);
                //force += g;
                newAcceleration = g / body->mass;
                //avgAcceleration = (lastAcceleration + newAcceleration) * 0.5;
                avgAcceleration = newAcceleration;
                body->velocity.setX(force.x());
                body->velocity.setZ(force.z());
                body->velocity += avgAcceleration * delta;

                // Gére le saut
                if (body->jump && body->onGround) {
                    body->velocity.setY(JUMP_SPEED);
                }
            } else {
                avgAcceleration = QVector3D(0.0f, 0.0f, 0.0f);
                body->velocity = force;
                newPosition += deltaPos;
            }

            body->acceleration = avgAcceleration;
            body->jump = false;

            body->position = newPosition;

            // Gestion du passage sous la map
            if (m_hasGravity && ((body->position.y() + body->height) < 0.0f)) {
                body->position.setY(CHUNK_SIZE * CHUNK_SCALE * 7);
            }
            // Gestion du blocage
            QVector3D voxel = body->position / (CHUNK_SCALE);
            voxel = QVector3D(floor(voxel.x()), floor(voxel.y()), floor(voxel.z()));
            if (m_hasGravity && (gl->getChunkManager().getVoxel(voxel.x(), voxel.y(), voxel.z()).type != VoxelType::AIR)) {
                body->position.setY(CHUNK_SIZE * CHUNK_SCALE * 7);
            }
        }
    }
}

bool PhysicManager::collide(GameWindow* gl, Body* body, QVector3D& position, const QVector3D& delta) {
    const float PADDING = 0.00001f;
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

    for (int i = 0; i < 8; ++i) {
        QVector3D oldVoxel = (corners[i] + position) / (CHUNK_SCALE);
        oldVoxel = QVector3D(floor(oldVoxel.x()), floor(oldVoxel.y()), floor(oldVoxel.z()));
        QVector3D newVoxel = (corners[i] + position + delta) / (CHUNK_SCALE);
        newVoxel = QVector3D(floor(newVoxel.x()), floor(newVoxel.y()), floor(newVoxel.z()));
        QVector3D direction = oldVoxel - newVoxel;
        direction.normalize();
        QVector3D currentVoxel = oldVoxel;
        if (oldVoxel != newVoxel) {
            do {
                currentVoxel -= direction;
                if (chunkManager.getVoxel(currentVoxel.x(), currentVoxel.y(), currentVoxel.z()).type != VoxelType::AIR) {
                    isColliding = true;
                    break;
                }
            } while (currentVoxel != newVoxel);
        }
    }
    if (!isColliding) {
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

