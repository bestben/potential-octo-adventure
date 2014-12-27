#include "camera.h"

#include <cmath>
#include <QKeyEvent>

#include "physic/body.h"
#include "gamewindow.h"

static const float pi = 4 * std::atan(1);

float degToRad(float x) {
    return x * pi / 180.0f;
}

float radToDeg(float x) {
    return x * 180.0f / pi;
}

Camera::Camera() : m_speed{100.0f}, m_phi{degToRad(-33.0f)}, m_theta{degToRad(-10.0f)},
                    m_thetaMax{degToRad(75.0f)}, m_sensi{0.5f}, m_fov{60.0f}, m_near{0.25f},
                    m_far{2500.0f}, m_width{1.0f}, m_height{1.0f},
                    m_direction{Direction::NONE}, m_mousePressed{false}, m_isViewMatrixDirty{true}, m_isProjMatrixDirty{false} {
    m_tang = (float)std::tan(m_fov * pi  / 180.0f);
    m_nh = m_near * m_tang;
    m_nw = m_nh * m_width / m_height;
    m_fh = m_far * m_tang;
    m_fw = m_fh * m_width / m_height;

}

void Camera::init(GameWindow* gl) {
    int bodyID = gl->getPhysicManager().allocBody();
    m_body = gl->getPhysicManager().getBody(bodyID);

    m_body->position = QVector3D(0.0f, 1085.0f, 0.0f);
    setCamDef(m_body->position, m_body->position + frontDir(), QVector3D(0.0f, 1.0f, 0.0f));
}

void Camera::update(int dt) {
    // On translate la caméra
    float mul = (dt / 1000.0f) * m_speed;
    QVector3D dir = getDirection();
    QVector3D move = dir * m_speed;

    m_body->force = move;
}

void Camera::postUpdate() {
    QVector3D viewDir = frontDir();
    viewDir.normalize();

    setCamDef(m_body->position, viewDir, QVector3D(0.0f, 1.0f, 0.0f));

    m_isViewMatrixDirty = true;
}

void Camera::setPosition(const QVector3D& v) {
    m_body->position = v;

    m_isViewMatrixDirty = true;
}

QVector3D Camera::getPosition() const {
    return m_body->position;
}

QVector3D Camera::getDirection() {
    QVector3D frontdir = frontDir();
    QVector3D rightdir = QVector3D::crossProduct(frontDir(), QVector3D{0.0f, 1.0f, 0.0f});

    QVector3D d;

    switch (m_direction) {
    case Direction::NONE:
        return {};
        break;
    case Direction::UP:
        return frontdir;
        break;
    case Direction::DOWN:
        return frontdir * -1;
        break;
    case Direction::LEFT:
        return rightdir * -1;
        break;
    case Direction::RIGHT:
        return rightdir;
        break;
    case Direction::UP_RIGHT:
        d = (frontdir + rightdir);
        d.normalize();
        return d;
        break;
    case Direction::UP_LEFT:
        d = (frontdir + rightdir * -1);
        d.normalize();
        return d;
        break;
    case Direction::DOWN_RIGHT:
        d = (frontdir * -1 + rightdir);
        d.normalize();
        return d;
        break;
    case Direction::DOWN_LEFT:
        d = (frontdir + rightdir) * -1;
        d.normalize();
        return d;
        break;
    }
    return {0.0f, 0.0f, 0.0f};
}

const QMatrix4x4& Camera::getViewMatrix() {
    // Si la caméra a été modifiée on re-calcule la matrice
    if (m_isViewMatrixDirty) {
        QVector3D front = frontDir();
        front.normalize();

        QVector3D eyePos = m_body->position + QVector3D(0.0f, m_body->height, 0.0f);
        QVector3D to = eyePos + front;

        m_viewMatrix.setToIdentity();
        m_viewMatrix.lookAt(eyePos, to, QVector3D(0.0, 1.0, 0.0));
        m_isViewMatrixDirty = false;
    }
    return m_viewMatrix;
}

const QMatrix4x4& Camera::getProjectionMatrix() {
    if (m_isProjMatrixDirty) {
        m_projMatrix.setToIdentity();
        m_projMatrix.perspective(m_fov, m_width / m_height, m_near, m_far);
        m_isProjMatrixDirty = false;
    }
    return m_projMatrix;
}

const QMatrix4x4& Camera::getViewProjMatrix() {
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

void Camera::setCamDef(const QVector3D &p, const QVector3D &l, const QVector3D &u) {

    QVector3D nc,fc,X,Y,Z;

    Z = l * -1.0;
    Z.normalize();

    X = QVector3D::crossProduct(u, Z);
    X.normalize();

    Y = QVector3D::crossProduct(Z, X);

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

    QVector3D aux,normal;
    // TOP
    aux = (nc + Y*m_nh) - p;
    aux.normalize();
    normal = QVector3D::crossProduct(aux, X);
    normal.normalize();
    m_planesOrigin[2] = normal;
    m_planesNormal[2] = nc+Y*m_nh;
    // BOTTOM
    aux = (nc - Y*m_nh) - p;
    aux.normalize();
    normal = QVector3D::crossProduct(X, aux);
    normal.normalize();
    m_planesOrigin[3] = normal;
    m_planesNormal[3] = nc-Y*m_nh;
    // LEFT
    aux = (nc - X*m_nw) - p;
    aux.normalize();
    normal = QVector3D::crossProduct(aux, Y);
    normal.normalize();
    m_planesOrigin[4] = normal;
    m_planesNormal[4] = nc-X*m_nw;
    // RIGHT
    aux = (nc + X*m_nw) - p;
    aux.normalize();
    normal = QVector3D::crossProduct(Y, aux);
    normal.normalize();
    m_planesOrigin[5] = normal;
    m_planesNormal[5] = nc+X*m_nw;
}

bool Camera::sphereInFrustum(const QVector3D& p, float radius) {
    float distance;
    bool result = true;

    for (int i = 0; i < 6; i++) {
        distance = p.distanceToPlane(m_planesOrigin[i], m_planesNormal[i]);
        if (distance < -radius) {
            return false;
        }
    }
    return result;
}

bool Camera::boxInFrustum(int x, int y, int z, int size) {
    bool allOut = true;

    // for each plane do ...
    for(int i = 0; i < 6; i++) {
        allOut = true;
        for (int k = 0; k < 8; k++) {
            int cx = (k >> 2) & 1;
            int cy = (k >> 1) & 1;
            int cz = (k >> 0) & 1;

            QVector3D c(x + cx * size, y + cy * size, z + cz * size);
            if (c.distanceToPlane(m_planesOrigin[i], m_planesNormal[i]) >= 0) {
                allOut = false;
                break;
            }
        }
        //if all corners are out
        if (allOut)
            return false;
    }
    return true;
 }

void Camera::keyPressEvent(QKeyEvent* event) {

    if (event->key() == Qt::Key_Space) {
        m_body->jump = true;
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
    if (m_mousePressed) {
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

////////////////////////////////////////////////////////////////////////////////
// FONCTIONS PRIVEES
////////////////////////////////////////////////////////////////////////////////

QVector3D Camera::frontDir() {
    QVector3D v{cos(m_theta) * cos(m_phi),
                sin(m_theta),
                -cos(m_theta) * sin(m_phi)};
    return v;
}
