#include "physicmanager.h"

#include <iostream>

#include "../gamewindow.h"

#include "body.h"

#define JUMP_SPEED 150

PhysicManager::PhysicManager() : m_freeBodies(BODY_COUNT, true), m_hasGravity{true} {
    m_bodies = new Body[BODY_COUNT];
    for (int i = 0; i < BODY_COUNT; ++i) {
        m_bodies[i].jump = false;
        m_bodies[i].height = 12;
        m_bodies[i].width = 1;
        m_bodies[i].mass = 1;
        m_bodies[i].jumpSpeed = JUMP_SPEED;
        m_bodies[i].onGround = false;
        m_bodies[i].inWater = false;
        m_bodies[i].isFullyInWater = false;
    }
}

PhysicManager::~PhysicManager() {
    delete[] m_bodies;
}

void PhysicManager::update(GameWindow* gl, int dt) {
    float delta = (float)dt / 1000.0;

    QVector3D g(0.0, -981.0f, 0.0);

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
                newAcceleration = g / body->mass;
                if (body->inWater) {
                    newAcceleration /= 5.0f;
                    force = force / 3.0f;
                } else {
                    newAcceleration /= 2.0f;
                }
                //avgAcceleration = (lastAcceleration + newAcceleration) * 0.5;
                avgAcceleration = newAcceleration;
                body->velocity.setX(force.x());
                body->velocity.setZ(force.z());
                body->velocity += avgAcceleration * delta;

                // Gére le saut
                if (body->jump && body->onGround && !body->inWater) {
                    body->velocity.setY(body->jumpSpeed);
                } else if (body->jump && body->inWater) {
                    body->velocity.setY(body->jumpSpeed / 2.0f);
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
				body->position.setY(CHUNK_SIZE * CHUNK_SCALE * WORLD_HEIGHT);
            }
            // Gestion du blocage
            QVector3D footVoxel = body->position / (CHUNK_SCALE);
            footVoxel = QVector3D(floor(footVoxel.x()), floor(footVoxel.y()), floor(footVoxel.z()));
            VoxelType type = gl->getChunkManager().getVoxel(footVoxel.x(), footVoxel.y(), footVoxel.z()).type;
			if (m_hasGravity && (type != VoxelType::AIR) && (type != VoxelType::WATER) && (type != VoxelType::IGNORE_TYPE)) {
                body->position.setY(CHUNK_SIZE * CHUNK_SCALE * WORLD_HEIGHT);
            }
            body->inWater = type == VoxelType::WATER;

            QVector3D headVoxel = (body->position + QVector3D(0.0f, body->height, 0.0f)) / (CHUNK_SCALE);
            headVoxel = QVector3D(floor(headVoxel.x()), floor(headVoxel.y()), floor(headVoxel.z()));
            type = gl->getChunkManager().getVoxel(headVoxel.x(), headVoxel.y(), headVoxel.z()).type;
            body->isFullyInWater = type == VoxelType::WATER;
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
                VoxelType type = chunkManager.getVoxel(currentVoxel.x(), currentVoxel.y(), currentVoxel.z()).type;
				if ((type != VoxelType::AIR) && (type != VoxelType::WATER) && (type != VoxelType::IGNORE_TYPE)) {
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

