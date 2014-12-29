#ifndef PLAYER_H
#define PLAYER_H

class GameWindow;
class Camera;

class QKeyEvent;
class QMouseEvent;

/**
 * @brief Classe s'occupant de la gestion du joueur.
 */
class Player {
public:
    Player(GameWindow& game, Camera& camera);
    ~Player();

    void update(int dt);

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

    // La distance max à laquelle le joueur peut poser / retirer un bloc
    float m_maxBlockDistance;
};

#endif // PLAYER_H
