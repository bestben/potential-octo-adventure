#pragma once

#include "wireframebox.h"
#include "voxelbuffer.h"
#include "particlesystem.h"
#include "chunk.h"

#include <QTime>

class GameWindow;
class Camera;
class QOpenGLTexture;

class QKeyEvent;
class QMouseEvent;

#define CROSS_WIDTH 72.0f
#define CROSS_HEIGHT 72.0f

/**
 * @brief Classe s'occupant de la gestion du joueur.
 */
class Player {
public:
    Player(GameWindow& game, Camera& camera);
    ~Player();

    void init();
    void destroy();
    void update(int dt);
    void draw();
    /**
     * @brief Affiche les informations extérieures à la scène.
     */
    void postDraw();

    /*
     * Fonctions gérant les événements.
     */
    void keyPressEvent(QKeyEvent* event);
    void keyReleaseEvent(QKeyEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent * event);

private:
    GameWindow& m_game;
    // La camera attachée au joueur
    Camera& m_camera;
    // La box à afficher autours du bloc selectionné
    WireframeBox m_box;
    // La box à afficher autours du bloc en cours de demolition
    VoxelBuffer m_voxel;
    // Les particules à afficher lorsque le joueur casse un bloc
    ParticleSystem m_particles;
    // La distance max à laquelle le joueur peut poser / retirer un bloc
    float m_maxBlockDistance;

    // Le shader affichant la cible du joueur.
    QOpenGLShaderProgram* m_crossProgram;
    // La texture de cible
    QOpenGLTexture* m_crossTexture;
    QOpenGLVertexArrayObject* m_crossVao;
    int m_crossXSizeUniform;
    int m_crossYSizeUniform;

    // Le joueur est-il en train de taper ?
    bool m_isHitting;
    // Le voxel visé
    Coords m_targetVoxel;
    // Le temps nécessaire pour casser le voxel
    int m_targetTime;
    // Le timer comptant le temps passé depuis que le joueur a commencé à taper le voxel
    QTime m_startTimer;
};
