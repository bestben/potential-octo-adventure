#pragma once

#include "glm/vec3.hpp"

/**
 * @brief Structure décrivant un body.
 * Un body est représenté par une boite alignée aux axes.
 */
struct Body {
    glm::vec3 position; // La position du body
    glm::vec3 velocity; // La vitesse du body
    glm::vec3 acceleration; //L'acceleration du body

    glm::vec3 force; // Une force à appliquer

    float height; // La hauteur du body
    float width; // La largeur du body
    float mass; // La masse du body
    float jumpSpeed; // La vitesse de saut
    bool jump; // Le body doit-il sauter ?
    bool onGround; // Le body touche-t-il le sol ?
    bool inWater; // Le body est-il dans l'eau ?
    bool isFullyInWater; // Le body est-il entièrement dans l'eau ?
};
