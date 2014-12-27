#pragma once

#include <vector>

#define BODY_COUNT 100

struct Body;
class GameWindow;
class QVector3D;

class PhysicManager {
public:
    PhysicManager();
    ~PhysicManager();

    void update(GameWindow* gl, int dt);

    int allocBody();
    void freeBody(int bodyIndex);

    Body* getBody(int bodyIndex);

    void setGravity(bool active);

private:
    bool collide(GameWindow* gl, Body* body, QVector3D& position, const QVector3D& delta);

    Body* m_bodies;
    std::vector<bool> m_freeBodies;

    bool m_hasGravity;
};
