#pragma once

#include "../wireframebox.h"
#include <vector>

#include "../chunk.h"

class GameWindow;
struct Body;
class Pathfinding;

class Npc {
public:
    virtual ~Npc() {}

    virtual void init(GameWindow* gl) = 0;
    virtual void destroy(GameWindow* gl) = 0;
    virtual void update(GameWindow* gl, int dt) = 0;
    virtual void draw(GameWindow* gl) = 0;

    virtual bool isDead() = 0;
    virtual bool canBeDestroyed() = 0;

    virtual void setPosition(const glm::vec3& pos) = 0;

private:

};

