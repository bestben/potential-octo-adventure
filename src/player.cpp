#include "player.h"

#include "gamewindow.h"
#include "camera.h"

#include <glad/glad.h>
#include <QtGui/QMouseEvent>
#include "utilities/openglprogramshader.h"
#include "utilities/opengltexture.h"
#include "utilities/openglvertexarrayobject.h"

#define GLM_FORCE_PURE
#include "glm/geometric.hpp"

Player::Player(GameWindow& game, Camera& camera) : m_game{game}, m_camera{camera}, m_particles{20, 750},
                                                  m_maxBlockDistance{40.0f}, m_isHitting{false}, m_targetTime{2000} {

}

Player::~Player() {
}

void Player::init() {
    m_box.init(&m_game);
    m_voxel.init(&m_game);

    m_crossProgram = std::make_unique<OpenglProgramShader>();
    m_crossProgram->addShaderFromSourceFile(OpenGLShaderType::Vertex, "shaders/cross.vs");
    m_crossProgram->addShaderFromSourceFile(OpenGLShaderType::Fragment, "shaders/cross.ps");
    m_crossProgram->link();
    m_crossProgram->bind();
    m_crossProgram->setUniformValue("texture", 0);
    m_crossProgram->release();

    m_crossXSizeUniform = m_crossProgram->uniformLocation("xSize");
    m_crossYSizeUniform = m_crossProgram->uniformLocation("ySize");

    m_crossTexture = std::make_unique<OpenGLTexture>("textures/cross.png");
    m_crossTexture->setMinMagFilters(OpenGLTexture::Linear, OpenGLTexture::Linear);

    m_crossVao = std::make_unique<OpenGLVertexArrayObject>();
    m_crossVao->create();

    m_particles.init(&m_game);
}

void Player::destroy() {
    m_crossProgram = nullptr;
    m_crossTexture = nullptr;
    m_crossVao = nullptr;
    m_box.destroy(&m_game);
    m_voxel.destroy(&m_game);
    m_particles.destroy(&m_game);
}

void Player::update(int dt) {
    m_particles.update(&m_game, dt);
}

void Player::draw() {
    glm::vec3 camPos = m_camera.getPosition();
    glm::vec3 dir = glm::normalize(m_camera.frontDir());
    float delta = 0.5f;
    float maxSquareDistance = m_maxBlockDistance * m_maxBlockDistance;

    Coords startVoxel = GetVoxelPosFromWorldPos(camPos);
    Coords lastVoxel = startVoxel;
    Coords currentVoxel = startVoxel;

    VoxelType type;

    glm::vec3 rayPos = camPos + dir * delta;
    ChunkManager& chunkManager = m_game.getChunkManager();
    bool hit = false;
    while (lengthSquare(rayPos - camPos) < maxSquareDistance) {
		currentVoxel = GetVoxelPosFromWorldPos(rayPos);
        type = chunkManager.getVoxel(currentVoxel.i, currentVoxel.j, currentVoxel.k).type;
        if ((type != VoxelType::AIR) && (type != VoxelType::WATER) && (type != VoxelType::IGNORE_TYPE)) {
            hit = true;
            break;
        }
        lastVoxel = currentVoxel;
        rayPos = rayPos + dir * delta;
    }
    if (!(lastVoxel == startVoxel) && hit) { // Le joueur a un voxel dans sa cible
        m_box.setPosition(glm::vec3((float)currentVoxel.i, (float)currentVoxel.j, (float)currentVoxel.k) * (float)CHUNK_SCALE);
        m_box.draw(&m_game);

        if (currentVoxel != m_targetVoxel) { // Le voxel est différent du précédant voxel visé
            m_targetVoxel = currentVoxel;
            m_startTimer.start();
        }
        if (m_isHitting) { // Le joueur est en train de taper
            if (m_startTimer.elapsed() >= m_targetTime) {
                chunkManager.removeVoxel({ currentVoxel.i, currentVoxel.j, currentVoxel.k });
            } else {
                glBlendFunc(GL_DST_COLOR, GL_SRC_COLOR);
                m_voxel.setDamage((float)m_startTimer.elapsed() / (float)m_targetTime);
                m_voxel.setPosition(glm::vec3((float)currentVoxel.i, (float)currentVoxel.j, (float)currentVoxel.k) * (float)CHUNK_SCALE);
                m_voxel.draw(&m_game);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            }
            m_particles.setVoxelType(type);
            m_particles.setSpawnPosition(voxelToWorld(currentVoxel));
            if (m_particles.isOver()) {
                m_particles.spawn();
            }
        }
    }

    m_particles.draw(&m_game);
}

void Player::postDraw() {
    m_crossProgram->bind();
    m_crossProgram->setUniformValue(m_crossXSizeUniform, CROSS_WIDTH / m_game.width());
    m_crossProgram->setUniformValue(m_crossYSizeUniform, CROSS_HEIGHT / m_game.height());
    m_crossTexture->bind(0);
    m_crossVao->bind();
    glDrawArrays(GL_TRIANGLES, 0, 6);
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
    if (event->button() == Qt::LeftButton) {
        m_isHitting = true;
        m_startTimer.start();
    }
    if (event->button() & Qt::MiddleButton) {
        glm::vec3 camPos = m_camera.getPosition();
        glm::vec3 dir = glm::normalize(m_camera.frontDir());
        float delta = 0.5f;
        float maxSquareDistance = m_maxBlockDistance * m_maxBlockDistance;

		Coords startVoxel = GetVoxelPosFromWorldPos(camPos);
        Coords lastVoxel = startVoxel;
        Coords currentVoxel = startVoxel;

        glm::vec3 rayPos = camPos + dir * delta;
        ChunkManager& chunkManager = m_game.getChunkManager();
        bool hit = false;
        while (lengthSquare(rayPos - camPos) < maxSquareDistance) {
			currentVoxel = GetVoxelPosFromWorldPos(rayPos);
            VoxelType type = chunkManager.getVoxel(currentVoxel.i, currentVoxel.j, currentVoxel.k).type;
			if ((type != VoxelType::AIR) && (type != VoxelType::WATER) && (type != VoxelType::IGNORE_TYPE)) {
                hit = true;
                break;
            }
            lastVoxel = currentVoxel;
            rayPos = rayPos + dir * delta;
        }
        if (!(lastVoxel == startVoxel) && hit) { // Le joueur a un voxel dans sa cible
            // Le voxel est différent du voxel sur lequel il se tient
            if (lastVoxel != GetVoxelPosFromWorldPos(m_camera.getFootPosition()) &&
                lastVoxel != GetVoxelPosFromWorldPos(m_camera.getPosition())) {

                Voxel last = chunkManager.getVoxel(lastVoxel);
                Voxel current = chunkManager.getVoxel(currentVoxel);
                chunkManager.placeVoxel({ lastVoxel.i, lastVoxel.j, lastVoxel.k }, VoxelType::DIRT);
            }
        }
    }
}

void Player::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_isHitting = false;
        m_voxel.setDamage(0.0f);
    }
    m_camera.mouseReleaseEvent(event);
}

void Player::mouseMoveEvent(QMouseEvent * event) {
    m_camera.mouseMoveEvent(event);
}
