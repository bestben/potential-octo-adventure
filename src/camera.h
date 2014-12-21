#pragma once

#include <QtGui/QWindow>
#include <QtGui/QMatrix4x4>
#include <QtGui/QVector3D>

/**
 * Enumeration permettant de savoir dans quelle direction avancer.
 */
enum Direction {
    UP = 1,
    DOWN = 2,
    RIGHT = 4,
    LEFT = 8,
    UP_RIGHT = 5,
    UP_LEFT = 9,
    DOWN_RIGHT = 6,
    DOWN_LEFT = 10,
    NONE = 0

};

/**
 * @brief Classe représentant une caméra pouvant naviguer dans un espace en 3D.
 */
class Camera {
public:
    Camera();

    /**
     * @brief Met à jour la caméra.
     * @param dt La durée écoulée depuis la dernière mise à jour.
     */
    void update(int dt);
    /**
     * @brief Modifie la position de la caméra.
     * @param v La nouvelle position de la caméra.
     */
    void setPosition(const QVector3D& v);
    /**
     * Renvoie la position de la camera.
     */
    QVector3D getPosition() const;
    /**
     * Renvoie la position précédante de la camera.
     */
    QVector3D getLastPosition() const;
    /**
     * Renvoie la direction de DEPLACEMENT de la caméra.
     * NOTE : La direction de déplacement peut être différente de la direction
     *         de VUE de la caméra.
     * @return Un vecteur normalisé.
     */
    QVector3D getDirection();
    /**
     * @brief Renvoie la matrice de vue de la caméra.
     */
    const QMatrix4x4& getViewMatrix();
    const QMatrix4x4& getProjectionMatrix();
    const QMatrix4x4& getViewProjMatrix();

    void changeViewportSize(int width, int height);

    bool sphereInFrustum(const QVector3D& p, float radius);
    bool boxInFrustum(int x, int y, int z, int size);

    /*
     * Fonctions gérant les événements.
     */
    void keyPressEvent(QKeyEvent* event);
    void keyReleaseEvent(QKeyEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent * event);

private:
    /*
     * Renvoie la direction dans laquelle la direction regarde.
     */
    QVector3D frontDir();

    void setCamDef(const QVector3D& p, const QVector3D& l, const QVector3D& u);

    float m_speed; // Vitesse de déplacement de la caméra (Unités / seconde).
    float m_phi;   // Orientation de la vue horizontale par rapport a x+ (en radian).
    float m_theta; // Orientation de la vue verticale par rapport a y+  (en radian).
    float m_thetaMax; // Angle theta maximum par rapport à (OZ) (en radian).
    float m_sensi; // Sensibilité de la souris (Degrés / pixel).

    float m_fov; // L'angle de vue
    float m_near;
    float m_far;
    float m_width;
    float m_height;

    QVector3D m_planesOrigin[6];
    QVector3D m_planesNormal[6];
    QVector3D m_ntl,m_ntr,m_nbl,m_nbr,
              m_ftl,m_ftr,m_fbl,m_fbr; // Les 4 coins de near et far
    float m_tang;
    float m_nw,m_nh,m_fw,m_fh;

    QVector3D m_position; // La position de la caméra
    QVector3D m_lastPosition;
    Direction m_direction; // La direction de déplacement de la caméra

    bool m_mousePressed; // Le clique droit est-il appuyé ?
    QPoint m_oldMousPos; // La dernière position de la souris

    bool m_isViewMatrixDirty; // La matrice de vue doit elle être recalculée ?
    bool m_isProjMatrixDirty; // La matrice de projection doit elle être recalculée ?
    QMatrix4x4 m_viewMatrix; // La matrice de vue
    QMatrix4x4 m_projMatrix; // La matrice de projection
    QMatrix4x4 m_viewProjMatrix; // La matrice de vue/projection
};
