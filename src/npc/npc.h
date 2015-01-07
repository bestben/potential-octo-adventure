#pragma once

#include "../wireframebox.h"
#include <vector>

#include "../chunk.h"

#include <QTimer>

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
    void updatePath();

    Body* m_body;

    Pathfinding* m_pathfinding;
    std::vector<Coords> m_path;
    Coords m_playerPosition;

    WireframeBox m_box;

    int m_lengthOfSight;
    int m_pathRefreshRate; // Le nombre de millisecondes entre chaque mise à jour du pathfinding
    QTimer m_refreshTimer;
};

