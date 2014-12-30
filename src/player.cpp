#include "player.h"

#include "gamewindow.h"
#include "camera.h"

#include <QtGui/QMouseEvent>
#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QOpenGLTexture>
#include <QtGui/QOpenGLVertexArrayObject>

Player::Player(GameWindow& game, Camera& camera) : m_game{game}, m_camera{camera}, m_maxBlockDistance{40.0f} {

}

Player::~Player() {
}

void Player::init() {
    m_box.init(&m_game);

    m_crossProgram = new QOpenGLShaderProgram(&m_game);
    m_crossProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, QString(":/cross.vs"));
    m_crossProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, QString(":/cross.ps"));
    m_crossProgram->link();
    m_crossProgram->bind();
    m_crossProgram->setUniformValue("texture", 0);
    m_crossProgram->release();

    m_crossXSizeUniform = m_crossProgram->uniformLocation("xSize");
    m_crossYSizeUniform = m_crossProgram->uniformLocation("ySize");

    m_crossTexture = new QOpenGLTexture(QImage(":/cross.png"));

    m_crossVao = new QOpenGLVertexArrayObject(&m_game);
    m_crossVao->create();
}

void Player::destroy() {
    delete m_crossProgram;
    delete m_crossTexture;
    m_box.destroy(&m_game);
}

void Player::update(int dt) {

}

void Player::draw() {
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
        m_box.setPosition(QVector3D(currentVoxel.i, currentVoxel.j, currentVoxel.k) * CHUNK_SCALE);
        m_box.draw(&m_game);
    }
}

void Player::postDraw() {
    m_crossProgram->bind();
    m_crossProgram->setUniformValue(m_crossXSizeUniform, CROSS_WIDTH / m_game.width());
    m_crossProgram->setUniformValue(m_crossYSizeUniform, CROSS_HEIGHT / m_game.height());
    m_crossTexture->bind(0);
    m_crossVao->bind();
    m_game.glDrawArrays(GL_TRIANGLES, 0, 6);
    m_crossVao->release();
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
