#pragma once

#include <QtGui/QVector3D>

struct Body {
    QVector3D position;
    QVector3D velocity;
    QVector3D acceleration;

    QVector3D force;

    float height;
    float width;
    float mass;
    float jumpSpeed;
    bool jump;
    bool onGround;
    bool inWater;
    bool isFullyInWater;
};
