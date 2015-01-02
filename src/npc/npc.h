#pragma once

#include "../wireframebox.h"
#include <vector>

#include "../chunk.h"

class GameWindow;
struct Body;
class Pathfinding;

class Npc {
public:
    Npc();
    ~Npc();

    void init(GameWindow* gl);
    void destroy(GameWindow* gl);
    void update(GameWindow* gl, int dt);
    void draw(GameWindow* gl);

private:
    Body* m_body;

    Pathfinding* m_pathfinding;
    std::vector<Coords> m_path;

    WireframeBox m_box;
};

