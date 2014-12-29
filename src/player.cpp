#include "player.h"

#include "gamewindow.h"
#include "camera.h"

#include <QtGui/QMouseEvent>
#include <iostream>

Player::Player(GameWindow& game, Camera& camera) : m_game{game}, m_camera{camera}, m_maxBlockDistance{40.0f} {

}

Player::~Player() {

}

void Player::update(int dt) {

}

void Player::keyPressEvent(QKeyEvent* event) {
    m_camera.keyPressEvent(event);
}

void Player::keyReleaseEvent(QKeyEvent* event) {
    m_camera.keyReleaseEvent(event);
}

void Player::mousePressEvent(QMouseEvent* event) {
    m_camera.mousePressEvent(event);
    if ((event->button() & Qt::MiddleButton) || (event->button() & Qt::LeftButton)) {
        QVector3D camPos = m_camera.getPosition();
        QVector3D dir = m_camera.frontDir();
        dir.normalize();
        float delta = 0.5f;
        float maxSquareDistance = m_maxBlockDistance * m_maxBlockDistance;

        Coords startVoxel = worldToVoxel(camPos);
        Coords lastVoxel = startVoxel;
        Coords currentVoxel = startVoxel;

        QVector3D rayPos = camPos + dir * delta;
        ChunkManager& chunkManager = m_game.getChunkManager();
        bool hit = false;
        while (((rayPos - camPos).lengthSquared() < maxSquareDistance)) {
            currentVoxel = worldToVoxel(rayPos);
            VoxelType type = chunkManager.getVoxel(currentVoxel.i, currentVoxel.j, currentVoxel.k).type;
            if ((type != VoxelType::AIR) && (type != VoxelType::WATER)) {
                hit = true;
                break;
            }
            lastVoxel = currentVoxel;
            rayPos = rayPos + dir * delta;
        }
        if (!(lastVoxel == startVoxel) && hit) {
            if (event->button() & Qt::LeftButton) {
                chunkManager.setVoxel(currentVoxel.i, currentVoxel.j, currentVoxel.k, VoxelType::AIR);
            } else {
                chunkManager.setVoxel(lastVoxel.i, lastVoxel.j, lastVoxel.k, VoxelType::GRASS);
            }
        }
    }
}

void Player::mouseReleaseEvent(QMouseEvent* event) {
    m_camera.mouseReleaseEvent(event);
}

void Player::mouseMoveEvent(QMouseEvent * event) {
    m_camera.mouseMoveEvent(event);
}
