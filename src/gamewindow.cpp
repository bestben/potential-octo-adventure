#include "gamewindow.h"

#include <iostream>
#include <QKeyEvent>

#include <QtCore/QCoreApplication>
#include <QtGui/QOpenGLDebugLogger>



GameWindow::GameWindow() : QOpenGLWindow(), m_player{*this, m_camera}, m_lastDelta{0},
                            m_currentDeltaIndex{0}, m_isInitialized{false}, m_chunkManager()
{
    m_deltaTimer.start();
    memset(m_lastDeltas, 0, FPS_FRAME_NUMBER * sizeof(int));

    m_hasPhysic = false;
}

GameWindow::~GameWindow() {
#ifdef QT_DEBUG
    delete m_logger;
#endif
    m_chunkManager.destroy(this);
    m_player.destroy();
}

void GameWindow::handleLoggedMessage(const QOpenGLDebugMessage& message) {
    std::cout << message.message().toStdString() << std::endl;
}

void GameWindow::initializeGL() {
    initializeOpenGLFunctions();
    
#ifdef QT_DEBUG
    m_logger = new QOpenGLDebugLogger(this);
    m_logger->initialize();
    connect(m_logger, &QOpenGLDebugLogger::messageLogged, this, &GameWindow::handleLoggedMessage, Qt::DirectConnection);
    m_logger->startLogging();
#endif

    const qreal retinaScale = devicePixelRatio();
    glViewport(0, 0, width() * retinaScale, height() * retinaScale);

    glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CW);

    glEnable(GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    m_camera.init(this);
    m_player.init();
    m_chunkManager.initialize(this);
    
    m_isInitialized = true;
}

void GameWindow::paintGL() {
	// Sky color
	glClearColor(.5f, .5f, .5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    
    m_lastDelta = m_deltaTimer.elapsed();
    m_deltaTimer.restart();
#ifdef QT_DEBUG
    m_lastDeltas[m_currentDeltaIndex++] = m_lastDelta;
    m_currentDeltaIndex = m_currentDeltaIndex % FPS_FRAME_NUMBER;
    setTitle(QString::number((int)getFPS()));
#endif

    // On met à jour la position de la caméra
    m_camera.update(m_lastDelta);
    m_physicManager.update(this, m_lastDelta);

    m_camera.postUpdate();
    m_player.update(m_lastDelta);

    m_chunkManager.update(this);
    m_chunkManager.draw(this);
    m_player.draw();

    // On demande une nouvelle frame
    QCoreApplication::postEvent(this, new QEvent(QEvent::UpdateRequest));
}

int GameWindow::getDeltaTime() {
    return m_lastDelta;
}

Camera& GameWindow::getCamera() {
    return m_camera;
}

void GameWindow::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Escape) {
        QCoreApplication::quit();
    } else if(event->key() == Qt::Key_P) {
        m_hasPhysic = !m_hasPhysic;
        m_physicManager.setGravity(m_hasPhysic);
    }
    m_player.keyPressEvent(event);
}

void GameWindow::keyReleaseEvent(QKeyEvent* event) {
    m_player.keyReleaseEvent(event);
}

void GameWindow::mousePressEvent(QMouseEvent* event) {
    m_player.mousePressEvent(event);
}

void GameWindow::mouseReleaseEvent(QMouseEvent* event) {
    m_player.mouseReleaseEvent(event);
}

void GameWindow::mouseMoveEvent(QMouseEvent * event) {
    m_player.mouseMoveEvent(event);
}

void GameWindow::resizeGL(int w, int h) {
    if (m_isInitialized) {
        const qreal retinaScale = devicePixelRatio();
        glViewport(0, 0, w * retinaScale, h * retinaScale);
    }
    m_camera.changeViewportSize(w, h);
}

float GameWindow::getFPS() const {
    float fps = 0;
    for (int i = 0; i < FPS_FRAME_NUMBER; ++i) {
        fps += m_lastDeltas[i];
    }
    return 1000.0 / (fps / (float)FPS_FRAME_NUMBER);
}

QOpenGLContext* GameWindow::getContext() const {
    return context();
}

ChunkManager& GameWindow::getChunkManager() {
    return m_chunkManager;
}

PhysicManager& GameWindow::getPhysicManager() {
    return m_physicManager;
}
