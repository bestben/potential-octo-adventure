#pragma once

#include <QtGui/QVector3D>

/**
 * @brief Structure décrivant un body.
 * Un body est représenté par une boite alignée aux axes.
 */
struct Body {
    QVector3D position; // La position du body
    QVector3D velocity; // La vitesse du body
    QVector3D acceleration; //L'acceleration du body

    QVector3D force; // Une force à appliquer

    float height; // La hauteur du body
    float width; // La largeur du body
    float mass; // La masse du body
    float jumpSpeed; // La vitesse de saut
    bool jump; // Le body doit-il sauter ?
    bool onGround; // Le body touche-t-il le sol ?
    bool inWater; // Le body est-il dans l'eau ?
    bool isFullyInWater; // Le body est-il entièrement dans l'eau ?
};
