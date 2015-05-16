#pragma once

#include <vector>

#include "npc.h"

#include "../wireframebox.h"
#include "../chunk.h"



class GameWindow;
struct Body;
class Pathfinding;

class Creeper : public Npc {
public:
    Creeper();
    ~Creeper();

    void init(GameWindow* gl);
    void destroy(GameWindow* gl);
    void update(GameWindow* gl, int dt);
    void draw(GameWindow* gl);

    bool isDead();
    bool canBeDestroyed();

    void setPosition(const glm::vec3& pos);

private:
    void updatePath();

    Body* m_body;

    Pathfinding* m_pathfinding;
    std::vector<Coords> m_path;
    Coords m_playerPosition;

    WireframeBox m_box;

    int m_lengthOfSight;
    int m_pathRefreshRate; // Le nombre de millisecondes entre chaque mise à jour du pathfinding
    //Timer m_refreshTimer;

    int m_life;
    bool m_canBeDestroyed;
};

