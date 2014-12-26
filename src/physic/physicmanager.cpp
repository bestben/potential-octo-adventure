#include "physicmanager.h"

#include <iostream>

#include "../gamewindow.h"

#include "body.h"

PhysicManager::PhysicManager() : m_freeBodies(BODY_COUNT, true), m_hasGravity{false} {
    m_bodies = new Body[BODY_COUNT];
    for (int i = 0; i < BODY_COUNT; ++i) {
        m_bodies[i].jump = false;
        m_bodies[i].height = 10;
        m_bodies[i].width = 5;
        m_bodies[i].mass = 5;
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
            QVector3D newPosition = body->position + body->velocity * delta + (0.5 * lastAcceleration * delta * delta);
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
                    force -= 2 * g;
                }
                newAcceleration = force / body->mass;
                avgAcceleration = (lastAcceleration + newAcceleration) * 0.5;
                body->velocity += avgAcceleration * delta;
            } else {
                avgAcceleration = QVector3D(0.0f, 0.0f, 0.0f);
                body->velocity = force;
            }

            body->acceleration = avgAcceleration;
            body->jump = false;

            // TODO Test collision ici

            body->position = newPosition;
        }
    }
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

