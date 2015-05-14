#include "camera.h"
#include "chunk.h"
#include <cmath>
#include <QKeyEvent>

#include "physic/body.h"
#include "gamewindow.h"

#define GLM_FORCE_PURE
#include "glm/geometric.hpp"
#include "glm/gtc/matrix_transform.hpp"

static const float pi = 4 * std::atan(1);

float degToRad(float x) {
    return x * pi / 180.0f;
}

float radToDeg(float x) {
    return x * 180.0f / pi;
}

Camera::Camera() : m_speed{CAMERA_WALK_SPEED}, m_phi{degToRad(-33.0f)}, m_theta{degToRad(-10.0f)},
                    m_thetaMax{degToRad(75.0f)}, m_sensi{0.5f}, m_fov{60.0f}, m_desiredFov{m_fov}, m_near{0.25f},
                    m_far{2500.0f}, m_width{1.0f}, m_height{1.0f},
                    m_direction{Direction::NONE}, m_mousePressed{false}, m_isViewMatrixDirty{true}, m_isProjMatrixDirty{false},
                    m_isFPS{false} {
    m_tang = (float)std::tan(m_fov * pi  / 180.0f);
    m_nh = m_near * m_tang;
    m_nw = m_nh * m_width / m_height;
    m_fh = m_far * m_tang;
    m_fw = m_fh * m_width / m_height;

}

void Camera::init(GameWindow* gl) {
    int bodyID = gl->getPhysicManager().allocBody();
    m_body = gl->getPhysicManager().getBody(bodyID);

    m_body->position = glm::vec3(124.0f, CHUNK_SCALE*CHUNK_SIZE*5.5f, 124.0f );
    setCamDef(m_body->position, m_body->position + frontDir(), glm::vec3(0.0f, 1.0f, 0.0f));
}

void Camera::update(GameWindow* gl, int dt) {
    if (m_isFPS) {
        QPoint cursorPos = gl->mapFromGlobal(QCursor::pos());
        int dx = cursorPos.x() - (m_width / 2);
        int dy = cursorPos.y() - (m_height / 2);

        m_phi -= degToRad(dx * m_sensi);
        m_theta -= degToRad(dy * m_sensi);

        /*
         * On évite à phi de devenir trop grand (ou trop petit)
         *   en enlevant (ou ajoutant) 1 tour à chaque tour
         */
        if (m_phi > degToRad(360.0f)) {
            m_phi -= degToRad(360.0f);
        } else if (m_phi < 0.0f) {
            m_phi += degToRad(360.0f);
        }
        // On évite que theta dépasse la limite
        if (m_theta > m_thetaMax) {
            m_theta = m_thetaMax;
        } else if (m_theta < -m_thetaMax) {
            m_theta = -m_thetaMax;
        }

        m_isViewMatrixDirty = true;
    }

    // On translate la caméra
    glm::vec3 dir = getDirection();
    glm::vec3 move = dir * m_speed;

    m_body->force = move;

    if (m_desiredFov > m_fov) {
        float df = CAMERA_FOV_SPEED * ((float)dt / 1000.0f);
        m_fov = std::min(m_fov + df, m_desiredFov);
        m_isProjMatrixDirty = true;
    } else {
        float df = CAMERA_FOV_RELEASE_SPEED * ((float)dt / 1000.0f);
        m_fov = std::max(m_fov - df, m_desiredFov);
        m_isProjMatrixDirty = true;
    }
}

void Camera::postUpdate() {
    glm::vec3 viewDir = glm::normalize(frontDir());

    setCamDef(m_body->position, viewDir, glm::vec3(0.0f, 1.0f, 0.0f));

    m_isViewMatrixDirty = true;
}

void Camera::setPosition(const glm::vec3& v) {
    m_body->position = v;

    m_isViewMatrixDirty = true;
}

glm::vec3 Camera::getPosition() const {
    return m_body->position + glm::vec3(0.0f, m_body->height, 0.0f);
}

glm::vec3 Camera::getFootPosition() const {
    return m_body->position;
}


glm::vec3 Camera::getDirection() {
    glm::vec3 frontdir = frontDir();
    glm::vec3 rightdir = glm::cross(frontDir(), glm::vec3(0.0f, 1.0f, 0.0f));

    glm::vec3 d;

    switch (m_direction) {
    case Direction::NONE:
        return {};
        break;
    case Direction::UP:
        return frontdir;
        break;
    case Direction::DOWN:
        return frontdir * -1.0f;
        break;
    case Direction::LEFT:
        return rightdir * -1.0f;
        break;
    case Direction::RIGHT:
        return rightdir;
        break;
    case Direction::UP_RIGHT:
        d = glm::normalize(frontdir + rightdir);
        return d;
        break;
    case Direction::UP_LEFT:
        d = glm::normalize(frontdir + rightdir * -1.0f);
        return d;
        break;
    case Direction::DOWN_RIGHT:
        d = glm::normalize(frontdir * -1.0f + rightdir);
        return d;
        break;
    case Direction::DOWN_LEFT:
        d = glm::normalize(frontdir + rightdir) * -1.0f;
        return d;
        break;
    }
    return {0.0f, 0.0f, 0.0f};
}

const glm::mat4x4& Camera::getViewMatrix() {
    // Si la caméra a été modifiée on re-calcule la matrice
    if (m_isViewMatrixDirty) {
        glm::vec3 front = glm::normalize(frontDir());

        glm::vec3 eyePos = m_body->position + glm::vec3(0.0f, m_body->height, 0.0f);
        glm::vec3 to = eyePos + front;

        m_viewMatrix = glm::lookAt(eyePos, to, glm::vec3(0.0, 1.0, 0.0));
        m_isViewMatrixDirty = false;
    }
    return m_viewMatrix;
}

const glm::mat4x4& Camera::getProjectionMatrix() {
    if (m_isProjMatrixDirty) {
        m_projMatrix = glm::perspective(degToRad(m_fov), m_width / m_height, m_near, m_far);
        m_isProjMatrixDirty = false;
    }
    return m_projMatrix;
}

const glm::mat4x4& Camera::getViewProjMatrix() {
    if (m_isProjMatrixDirty || m_isViewMatrixDirty) {
        m_viewProjMatrix = getProjectionMatrix() * getViewMatrix();
    }
    return m_viewProjMatrix;
}

void Camera::changeViewportSize(int width, int height) {
    m_width = width;
    m_height = height;

    m_nh = m_near * m_tang;
    m_nw = m_nh * m_width / m_height;
    m_fh = m_far * m_tang;
    m_fw = m_fh * m_width / m_height;

    m_isProjMatrixDirty = true;
}

void Camera::setCamDef(const glm::vec3 &p, const glm::vec3 &/*l*/, const glm::vec3 &u) {

    glm::vec3 nc,fc,X,Y,Z;

    Z = glm::normalize(glm::vec3(-1.0f, -1.0f, -1.0f));
    X = glm::normalize(glm::cross(u, Z));
    Y = glm::normalize(glm::cross(Z, X));

    nc = p - Z * m_near;
    fc = p - Z * m_far;

    m_ntl = nc + Y * m_nh - X * m_nw;
    m_ntr = nc + Y * m_nh + X * m_nw;
    m_nbl = nc - Y * m_nh - X * m_nw;
    m_nbr = nc - Y * m_nh + X * m_nw;

    m_ftl = fc + Y * m_fh - X * m_fw;
    m_ftr = fc + Y * m_fh + X * m_fw;
    m_fbl = fc - Y * m_fh - X * m_fw;
    m_fbr = fc - Y * m_fh + X * m_fw;

    // NEAR
    m_planesOrigin[0] = nc;
    m_planesNormal[0] = -Z;
    // FAR
    m_planesOrigin[1] = fc;
    m_planesNormal[1] = Z;

    glm::vec3 aux,normal;
    // TOP
    aux = glm::normalize((nc + Y*m_nh) - p);
    normal = glm::normalize(glm::cross(aux, X));

    m_planesOrigin[2] = normal;
    m_planesNormal[2] = nc+Y*m_nh;
    // BOTTOM
    aux = glm::normalize((nc - Y*m_nh) - p);
    normal = glm::normalize(glm::cross(X, aux));
    m_planesOrigin[3] = normal;
    m_planesNormal[3] = nc-Y*m_nh;
    // LEFT
    aux = glm::normalize((nc - X*m_nw) - p);
    normal = glm::normalize(glm::cross(aux, Y));
    m_planesOrigin[4] = normal;
    m_planesNormal[4] = nc-X*m_nw;
    // RIGHT
    aux = glm::normalize((nc + X*m_nw) - p);
    normal = glm::normalize(glm::cross(Y, aux));
    m_planesOrigin[5] = normal;
    m_planesNormal[5] = nc+X*m_nw;
}

bool Camera::sphereInFrustum(const glm::vec3& /*p*/, float /*radius*/) {
    return true;
    /*float distance;
    bool result = true;

    for (int i = 0; i < 6; i++) {
        distance = p.distanceToPlane(m_planesOrigin[i], m_planesNormal[i]);
        if (distance < -radius) {
            return false;
        }
    }
    return result;*/
}

bool Camera::boxInFrustum(int /*x*/, int /*y*/, int /*z*/, int /*size*/) {
    /*bool allOut = true;

    // for each plane do ...
    for(int i = 0; i < 6; i++) {
        allOut = true;
        for (int k = 0; k < 8; k++) {
            int cx = (k >> 2) & 1;
            int cy = (k >> 1) & 1;
            int cz = (k >> 0) & 1;

            glm::vec3 c(x + cx * size, y + cy * size, z + cz * size);
            if (c.distanceToPlane(m_planesOrigin[i], m_planesNormal[i]) >= 0) {
                allOut = false;
                break;
            }
        }
        //if all corners are out
        if (allOut)
            return false;
    }*/
    return true;
}

void Camera::keyPressEvent(QKeyEvent* event) {

    if (event->key() == Qt::Key_Space) {
        m_body->jump = true;
    } else if (event->key() == Qt::Key_Control) {
        m_speed = CAMERA_RUN_SPEED;
        m_desiredFov = CAMERA_RUN_FOV;
        m_isProjMatrixDirty = true;
    }

    Direction mod = Direction::NONE;
    if (event->key() == Qt::Key_Z) {
        mod = Direction::UP;
    } else if (event->key() == Qt::Key_S) {
        mod = Direction::DOWN;
    } else if (event->key() == Qt::Key_Q) {
        mod = Direction::LEFT;
    } else if (event->key() == Qt::Key_D) {
        mod = Direction::RIGHT;
    } else {
        mod = Direction::NONE;
    }
    m_direction = (Direction)((int)m_direction | (int)mod);
}

void Camera::keyReleaseEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Control) {
        m_speed = CAMERA_WALK_SPEED;
        m_desiredFov = CAMERA_WALK_FOV;
        m_isProjMatrixDirty = true;
    }
    Direction mod = Direction::NONE;
    if (event->key() == Qt::Key_Z) {
        mod = Direction::UP;
    } else if (event->key() == Qt::Key_S) {
        mod = Direction::DOWN;
    } else if (event->key() == Qt::Key_Q) {
        mod = Direction::LEFT;
    } else if (event->key() == Qt::Key_D) {
        mod = Direction::RIGHT;
    } else {
        mod = Direction::NONE;
    }

    m_direction = (Direction)((int)m_direction & (~(int)mod));
}

void Camera::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::MouseButton::RightButton) {
        m_mousePressed = true;
        m_oldMousPos = event->pos();
    }
}

void Camera::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::MouseButton::RightButton) {
        m_mousePressed = false;
    }
}

void Camera::mouseMoveEvent(QMouseEvent* event) {
    if (m_mousePressed && !m_isFPS) {
        float dx = event->x() - m_oldMousPos.x();
        float dy = event->y() - m_oldMousPos.y();
        m_oldMousPos = event->pos();
        m_phi -= degToRad(dx * m_sensi);
        m_theta -= degToRad(dy * m_sensi);

        /*
         * On évite à phi de devenir trop grand (ou trop petit)
         *   en enlevant (ou ajoutant) 1 tour à chaque tour
         */
        if (m_phi > degToRad(360.0f)) {
            m_phi -= degToRad(360.0f);
        } else if (m_phi < 0.0f) {
            m_phi += degToRad(360.0f);
        }
        // On évite que theta dépasse la limite
        if (m_theta > m_thetaMax) {
            m_theta = m_thetaMax;
        } else if (m_theta < -m_thetaMax) {
            m_theta = -m_thetaMax;
        }

        m_isViewMatrixDirty = true;
    }
}

bool Camera::isInWater() const {
    return m_body->isFullyInWater;
}

glm::vec3 Camera::frontDir() {
    glm::vec3 v{cos(m_theta) * cos(m_phi),
                sin(m_theta),
                -cos(m_theta) * sin(m_phi)};
    return v;
}

void Camera::setFPSMode(bool isFPS) {
    m_isFPS = isFPS;
}

bool Camera::isFPSMode() const {
    return m_isFPS;
}
