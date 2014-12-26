#pragma once

#include <vector>

#define BODY_COUNT 100

struct Body;
class GameWindow;

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
    Body* m_bodies;
    std::vector<bool> m_freeBodies;

    bool m_hasGravity;
};
