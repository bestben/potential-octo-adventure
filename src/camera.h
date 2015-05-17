#pragma once

#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/mat4x4.hpp"

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

struct Body;
class GameWindow;

#define CAMERA_WALK_SPEED 30.0f
#define CAMERA_RUN_SPEED 60.0f

#define CAMERA_WALK_FOV 60.0f
#define CAMERA_RUN_FOV 90.0f

#define CAMERA_FOV_SPEED 15.0f // La vitesse du changement de fov par second
#define CAMERA_FOV_RELEASE_SPEED 30.0f // La vitesse du changement de fov par second lors du relachement

/**
 * @brief Classe représentant une caméra pouvant naviguer dans un espace en 3D.
 */
class Camera {
public:
    Camera();

    void init(GameWindow* gl);

    /**
     * @brief Met à jour la caméra.
     * @param dt La durée écoulée depuis la dernière mise à jour.
     */
    void update(GameWindow* gl, int dt);
    /**
     * @brief Fonction de mise à jour après le calcule de la physique.
     */
    void postUpdate();
    /**
     * @brief Modifie la position de la caméra.
     * @param v La nouvelle position de la caméra.
     */
    void setPosition(const glm::vec3& v);
    /**
     * Renvoie la position de la camera.
     */
    glm::vec3 getPosition() const;

    glm::vec3 getFootPosition() const;
    /**
     * Renvoie la direction de DEPLACEMENT de la caméra.
     * NOTE : La direction de déplacement peut être différente de la direction
     *         de VUE de la caméra.
     * @return Un vecteur normalisé.
     */
    glm::vec3 getDirection();
    /*
     * Renvoie la direction dans laquelle la camera regarde.
     */
    glm::vec3 frontDir();
    /**
     * @brief Renvoie la matrice de vue de la caméra.
     */
    const glm::mat4x4& getViewMatrix();
    const glm::mat4x4& getProjectionMatrix();
    const glm::mat4x4& getViewProjMatrix();

    void changeViewportSize(int width, int height);

    bool sphereInFrustum(const glm::vec3& p, float radius);
    bool boxInFrustum(int x, int y, int z, int size);

    bool isInWater() const;
    void setFPSMode(bool isFPS);
    bool isFPSMode() const;

    /*
     * Fonctions gérant les événements.
     */
	void keyPressEvent(int key, int scancode, int action, int mods);
	void keyReleaseEvent(int key, int scancode, int action, int mods);
	void mousePressEvent(int button, int action, int mods, float xpos, float ypos);
	void mouseReleaseEvent(int button, int action, int mods);
	void mouseMoveEvent(float xpos, float ypos);

private:
    void setCamDef(const glm::vec3& p, const glm::vec3& l, const glm::vec3& u);

    float m_speed; // Vitesse de déplacement de la caméra (Unités / seconde).
    float m_phi;   // Orientation de la vue horizontale par rapport a x+ (en radian).
    float m_theta; // Orientation de la vue verticale par rapport a y+  (en radian).
    float m_thetaMax; // Angle theta maximum par rapport à (OZ) (en radian).
    float m_sensi; // Sensibilité de la souris (Degrés / pixel).

    float m_fov; // L'angle de vue
    float m_desiredFov;
    float m_near;
    float m_far;
    float m_width;
    float m_height;

    glm::vec3 m_planesOrigin[6];
    glm::vec3 m_planesNormal[6];
    glm::vec3 m_ntl,m_ntr,m_nbl,m_nbr,
              m_ftl,m_ftr,m_fbl,m_fbr; // Les 4 coins de near et far
    float m_tang;
    float m_nw,m_nh,m_fw,m_fh;

    Body* m_body;
    Direction m_direction; // La direction de déplacement de la caméra

    bool m_mousePressed; // Le clique droit est-il appuyé ?
    glm::vec2 m_oldMousPos; // La dernière position de la souris

    bool m_isViewMatrixDirty; // La matrice de vue doit elle être recalculée ?
    bool m_isProjMatrixDirty; // La matrice de projection doit elle être recalculée ?
    glm::mat4x4 m_viewMatrix; // La matrice de vue
    glm::mat4x4 m_projMatrix; // La matrice de projection
    glm::mat4x4 m_viewProjMatrix; // La matrice de vue/projection

    bool m_isFPS;
};
