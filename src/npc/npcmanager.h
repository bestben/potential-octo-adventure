#pragma once

#include <vector>

class GameWindow;
class Npc;

class NpcManager {
public:
    NpcManager();
    ~NpcManager();

    void init(GameWindow* game);
    void destroy(GameWindow* game);

    void update(GameWindow* game, int dt);
    void draw(GameWindow* game);

private:
    int getFreeIndex();

    std::vector<Npc*> m_npcs;
};
