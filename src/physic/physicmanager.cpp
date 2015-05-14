#include "physicmanager.h"

#include <iostream>

#include "../gamewindow.h"

#include "body.h"

#include "glm/geometric.hpp"
#include "glm/vec3.hpp"

// La vitesse de saut par défaut
#define JUMP_SPEED 150

PhysicManager::PhysicManager() : m_freeBodies(BODY_COUNT, true), m_hasGravity{true} {
    // Initialise tous les body
    m_bodies = new Body[BODY_COUNT];
    for (int i = 0; i < BODY_COUNT; ++i) {
        m_bodies[i].jump = false;
        m_bodies[i].height = 12;
        m_bodies[i].width = 2;
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

    glm::vec3 g(0.0, -981.0f, 0.0);

    for (int i = 0; i < BODY_COUNT; ++i) {
        if (!m_freeBodies[i]) {
            Body* body = m_bodies + i;
            glm::vec3 lastAcceleration = body->acceleration;

            glm::vec3 deltaPos = body->velocity * delta + (lastAcceleration * delta * delta * 0.5f);
            glm::vec3 newPosition = body->position;
            glm::vec3 newAcceleration;
            glm::vec3 avgAcceleration;

            glm::vec3 force = body->force;
            if (force.x == 0.0f) {
                lastAcceleration.x = 0.0f;
            }
            if (force.z == 0.0f) {
                lastAcceleration.z = 0.0f;
            }
            if (m_hasGravity) {
                // Test collision
                bool colliding = collide(gl, body, newPosition, glm::vec3(0.0f, deltaPos.y, 0.0f));
                if (colliding) {
                    avgAcceleration.y = 0.0f;
                    if (body->velocity.y < 0.0f) {
                        body->velocity.y = 0.0f;
                    }
                    m_bodies[i].onGround = true;
                } else {
                    m_bodies[i].onGround = false;
                }
                colliding = collide(gl, body, newPosition, glm::vec3(deltaPos.x, 0.0f, 0.0f));
                if (colliding) {
                    avgAcceleration.x = 0.0f;
                    body->velocity.x = 0.0f;
                }
                colliding = collide(gl, body, newPosition, glm::vec3(0.0f, 0.0f, deltaPos.z));
                if (colliding) {
                    avgAcceleration.z = 0.0f;
                    body->velocity.z = 0.0f;
                }

                // Gestion physique
                force.y = 0.0f;
                newAcceleration = g / body->mass;
                if (body->inWater) {
                    newAcceleration /= 5.0f;
                    force = force / 3.0f;
                } else {
                    newAcceleration /= 2.0f;
                }
                //avgAcceleration = (lastAcceleration + newAcceleration) * 0.5;
                avgAcceleration = newAcceleration;
                body->velocity.x = force.x;
                body->velocity.z = force.z;
                body->velocity += avgAcceleration * delta;

                // Gére le saut
                if (body->jump && body->onGround && !body->inWater) {
                    body->velocity.y = body->jumpSpeed;
                } else if (body->jump && body->inWater) {
                    body->velocity.y = body->jumpSpeed * 0.5f;
                }
            } else {
                avgAcceleration = glm::vec3(0.0f, 0.0f, 0.0f);
                body->velocity = force;
                newPosition += deltaPos;
            }

            body->acceleration = avgAcceleration;
            body->jump = false;

            body->position = newPosition;

            // Gestion du passage sous la map
            if (m_hasGravity && ((body->position.y + body->height) < 0.0f)) {
                body->position.y = CHUNK_SIZE * CHUNK_SCALE * WORLD_HEIGHT;
            }
            // Gestion du blocage
            glm::vec3 footVoxel = body->position / (float)(CHUNK_SCALE);
            footVoxel = glm::vec3(floor(footVoxel.x), floor(footVoxel.y), floor(footVoxel.z));
            VoxelType type = gl->getChunkManager().getVoxel(footVoxel.x, footVoxel.y, footVoxel.z).type;
			if (m_hasGravity && (type != VoxelType::AIR) && (type != VoxelType::WATER) && (type != VoxelType::IGNORE_TYPE)) {
                body->position.y = (float)(CHUNK_SIZE * CHUNK_SCALE * WORLD_HEIGHT);
            }
            body->inWater = type == VoxelType::WATER;

            glm::vec3 headVoxel = (body->position + glm::vec3(0.0f, body->height, 0.0f)) / (float)(CHUNK_SCALE);
            headVoxel = glm::vec3(floor(headVoxel.x), floor(headVoxel.y), floor(headVoxel.z));
            type = gl->getChunkManager().getVoxel(headVoxel.x, headVoxel.y, headVoxel.z).type;
            body->isFullyInWater = type == VoxelType::WATER;
        }
    }
}

bool PhysicManager::collide(GameWindow* gl, Body* body, glm::vec3& position, const glm::vec3& delta) {
    const float PADDING = 0.00001f;
    ChunkManager& chunkManager = gl->getChunkManager();
    // Tous les coins de notre boite
    glm::vec3 corners[] = {
        glm::vec3(body->width + PADDING, - PADDING, body->width + PADDING),
        glm::vec3(body->width + PADDING, - PADDING, -body->width - PADDING),
        glm::vec3(-body->width - PADDING, - PADDING, body->width + PADDING),
        glm::vec3(-body->width - PADDING, - PADDING, -body->width - PADDING),

        glm::vec3(body->width, body->height * 0.5f, body->width),
        glm::vec3(body->width, body->height * 0.5f, -body->width),
        glm::vec3(-body->width, body->height * 0.5f, body->width),
        glm::vec3(-body->width, body->height * 0.5f, -body->width),

        glm::vec3(body->width, body->height, body->width),
        glm::vec3(body->width, body->height, -body->width),
        glm::vec3(-body->width, body->height, body->width),
        glm::vec3(-body->width, body->height, -body->width)
    };

    bool isColliding = false;

    for (int i = 0; i < 12; ++i) {
        glm::vec3 oldVoxel = (corners[i] + position) / (float)(CHUNK_SCALE);
        oldVoxel = glm::vec3(floor(oldVoxel.x), floor(oldVoxel.y), floor(oldVoxel.z));
        glm::vec3 newVoxel = (corners[i] + position + delta) / (float)(CHUNK_SCALE);
        newVoxel = glm::vec3(floor(newVoxel.x), floor(newVoxel.y), floor(newVoxel.z));
        glm::vec3 direction = glm::normalize(oldVoxel - newVoxel);
        glm::vec3 currentVoxel = oldVoxel;
        if (oldVoxel != newVoxel) {
            do {
                currentVoxel -= direction;
                VoxelType type = chunkManager.getVoxel(currentVoxel.x, currentVoxel.y, currentVoxel.z).type;
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

