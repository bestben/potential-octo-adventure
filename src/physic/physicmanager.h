#pragma once

#include <vector>

#include "../glm/fwd.hpp"

#define BODY_COUNT 100

struct Body;
class GameWindow;

/**
 * @brief Classe gérant la physique et les collisions.
 */
class PhysicManager {
public:
    PhysicManager();
    ~PhysicManager();

    /**
     * @brief Met à jour tous les body.
     */
    void update(GameWindow* gl, float dt);

    /**
     * @brief Crée un nouveau body.
     * @return Renvoie l'identifiant du body (-1 en cas d'erreur).
     */
    int allocBody();
    /**
     * @brief Supprime un body.
     */
    void freeBody(int bodyIndex);
    /**
     * @brief Renvoie un body en fonction de son index.
     */
    Body* getBody(int bodyIndex);

    /**
     * @brief Active/désactive la gravité.
     */
    void setGravity(bool active);

private:
    bool collide(GameWindow* gl, Body* body, glm::vec3& position, const glm::vec3& delta);

    // Tableau des body
    Body* m_bodies;
    // Tableau gérant les body libre.
    std::vector<bool> m_freeBodies;

    bool m_hasGravity;
};
