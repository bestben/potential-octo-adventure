#pragma once

#include "camera.h"

#include <QTimer>
#include <QTime>

#include <QtGui/QOpenGLWindow>
#include <QtGui/QOpenGLFunctions_3_3_Core>

class QOpenGLTexture;
class QOpenGLDebugLogger;
class QOpenGLDebugMessage;
class Camera;

// Le nombre de frames à considérer pour le calcul des FPS
#define FPS_FRAME_NUMBER 100

class GameWindow : public QOpenGLWindow, public QOpenGLFunctions_3_3_Core {
public:
    /**
     * @brief Constructeur de la classe GameWindow.
     */
    GameWindow();
    ~GameWindow();
    /**
     * @brief Initialise la fenêtre.
     */
    void initializeGL();
    /**
     * @brief Redessine la fenêtre.
     */
    void paintGL();
    /**
     * @brief Renvoie le delta de la frame.
     */
    int getDeltaTime();

    void keyPressEvent(QKeyEvent* event);
    void keyReleaseEvent(QKeyEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void resizeGL(int w, int h);

    void handleLoggedMessage(const QOpenGLDebugMessage& message);

    QOpenGLContext* getContext() const;
    /**
     * @brief Renvoie la camera actuelle de la fenêtre.
     */
    Camera& getCamera();
    /**
     * @brief Renvoie le nombre de frames par seconde.
     */
    float getFPS() const;

private:
    Camera m_camera;
    QOpenGLDebugLogger* m_logger;

    int m_lastDelta;
    int m_currentDeltaIndex;
    QTimer m_timer;
    QTime m_deltaTimer;
    int m_lastDeltas[FPS_FRAME_NUMBER];
    bool m_isInitialized;
};
