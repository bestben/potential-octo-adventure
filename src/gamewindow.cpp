#include "gamewindow.h"

#include "utilities/vsDebugLib.h"

#include <glad/glad.h>
#include <glfw/glfw3.h>

#include <iostream>
#include <sstream>
#include <algorithm>
#include <thread>
#include <cstring>

#undef max

void window_size_callback(GLFWwindow* pWindow, int width, int height) {
	GameWindow* pGameWindow = (GameWindow*)glfwGetWindowUserPointer( pWindow );

	pGameWindow->resizeGL( width, height );
}

void key_callback(GLFWwindow* pWindow, int key, int scancode, int action, int mods)
{
	GameWindow* pGameWindow = (GameWindow*)glfwGetWindowUserPointer(pWindow);
	if (action == GLFW_PRESS)
		pGameWindow->keyPressEvent( key, scancode, action, mods );
	else if(action == GLFW_RELEASE)
		pGameWindow->keyReleaseEvent(key, scancode, action, mods);
}

void cursor_position_callback(GLFWwindow* pWindow, double xpos, double ypos)
{
	GameWindow* pGameWindow = (GameWindow*)glfwGetWindowUserPointer(pWindow);
	pGameWindow->mouseMoveEvent((float)xpos, (float)ypos);
}

void mouse_button_callback(GLFWwindow* pWindow, int button, int action, int mods)
{
	GameWindow* pGameWindow = (GameWindow*)glfwGetWindowUserPointer(pWindow);

	if (action == GLFW_PRESS) {
		double xpos, ypos;
		glfwGetCursorPos(pWindow, &xpos, &ypos);
		pGameWindow->mousePressEvent(button, action, mods, (float)xpos, (float)ypos);
	} else if (action == GLFW_RELEASE)
		pGameWindow->mouseReleaseEvent(button, action, mods);
}

GameWindow::GameWindow(int worldSeed) : 
		m_player{ *this, m_camera }, m_lastDelta{ 0 },
		m_currentDeltaIndex{ 0 }, m_isInitialized{ false }, 
		m_waterPostProcess{ "shaders/waterPostProcess.ps" }, 
		m_chunkManager(worldSeed)
{
    m_deltaTimer.start();
    memset(m_lastDeltas, 0, FPS_FRAME_NUMBER * sizeof(int));

    m_hasPhysic = false;
}

GameWindow::~GameWindow() {
    m_npcManager.destroy(this);
    m_waterPostProcess.destroy(this);
    m_framebuffer.destroy(this);
    m_chunkManager.destroy(this);
    m_player.destroy();

	glfwTerminate();
}

void GameWindow::init() {
	if (!glfwInit()) {
		MI_ASSERT(false);
	}

	glfwWindowHint(GLFW_DOUBLEBUFFER, GL_TRUE);
	glfwWindowHint(GLFW_REFRESH_RATE, 60);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
#ifdef MI_DEBUG
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwSwapInterval(1);

	/* Create a windowed mode window and its OpenGL context */
	m_window = glfwCreateWindow(640, 480, "VoxelWorld", NULL, NULL);
	if (!m_window)
	{
		std::cerr << "Impossible de trouver une version d'opengl compatible" << std::endl;
		glfwTerminate();
		MI_ASSERT(false);
		exit(-1);
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(m_window);
	if (!gladLoadGL()) {
		MI_ASSERT(false);
	}

#ifdef MI_DEBUG
	VSDebugLib::init();
#endif
	int width, height;
	glfwGetFramebufferSize(m_window, &width, &height);
	glViewport(0, 0, width, height);

    glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CW);

    glEnable(GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    initializeTextureMaps();

    m_camera.init(this);
	int w, h;
	glfwGetFramebufferSize(m_window, &w, &h);
	m_camera.changeViewportSize(w, h);
    m_player.init();
    m_chunkManager.initialize(this);
    m_framebuffer.initialize(this);
    m_waterPostProcess.init(this);
    
	if (m_camera.isFPSMode())
		glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	else
		glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    m_npcManager.init(this);

    m_isInitialized = true;

	glfwSetWindowUserPointer(m_window, this);
	glfwSetFramebufferSizeCallback(m_window, window_size_callback);
	glfwSetKeyCallback(m_window, key_callback);
	glfwSetCursorPosCallback(m_window, cursor_position_callback);
	glfwSetMouseButtonCallback(m_window, mouse_button_callback);
}

void GameWindow::run() {
	float fLastRender = 66;
	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(m_window))
	{
		m_lastDelta = m_deltaTimer.elapsed();
		fLastRender += m_lastDelta;
		m_deltaTimer.restart();
#ifdef MI_DEBUG
		m_lastDeltas[m_currentDeltaIndex++] = m_lastDelta;
		m_currentDeltaIndex = m_currentDeltaIndex % FPS_FRAME_NUMBER;
		std::stringstream ss;
		ss << (int)getFPS();
		glfwSetWindowTitle(m_window, ss.str().c_str());
#endif

		// On met à jour la position de la caméra
		m_camera.update(this, m_lastDelta);
		m_npcManager.update(this, m_lastDelta);
		m_physicManager.update(this, m_lastDelta);

		m_camera.postUpdate();
		m_player.update(m_lastDelta);
		m_chunkManager.update(this);

		if (fLastRender >= 33.0f)
		{
			/* Render here */
			render();
			/* Swap front and back buffers */
			glfwSwapBuffers(m_window);
			fLastRender = 0;
		} //else
			//std::this_thread::sleep_for(std::chrono::milliseconds(1));

		/* Poll for and process events */
		glfwPollEvents();

		//std::this_thread::sleep_for(std::chrono::milliseconds(std::max(16 - m_lastDelta, 0)));
	}
}

void GameWindow::render() {
	// Sky color
	glm::vec3 skyColor(0.53f, 0.807f, 0.92);
    glClearColor(skyColor.x, skyColor.y, skyColor.z, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // On commence l'affichage
    m_framebuffer.begin(this);
    m_chunkManager.draw(this);
    m_player.draw();
    m_npcManager.draw(this);
    if (m_camera.isInWater()) {
        m_waterPostProcess.render(this);
    }
    m_framebuffer.end(this);

    m_player.postDraw();
}

int GameWindow::getDeltaTime() {
    return m_lastDelta;
}

Camera& GameWindow::getCamera() {
    return m_camera;
}

void GameWindow::keyPressEvent(int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE) {
		glfwSetWindowShouldClose(m_window, GL_TRUE);
	} else if (key == GLFW_KEY_P) {
        m_hasPhysic = !m_hasPhysic;
        m_physicManager.setGravity(m_hasPhysic);
	} else if (key == GLFW_KEY_F) {
        bool fps = m_camera.isFPSMode();
        fps = !fps;
        m_camera.setFPSMode(fps);
        if (fps)
			glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		else
			glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    m_player.keyPressEvent( key, scancode, action, mods);
}

void GameWindow::keyReleaseEvent(int key, int scancode, int action, int mods) {
	m_player.keyReleaseEvent(key, scancode, action, mods);
}

void GameWindow::mousePressEvent(int button, int action, int mods, float xpos, float ypos) {
    m_player.mousePressEvent(button, action, mods, xpos, ypos);
}

void GameWindow::mouseReleaseEvent(int button, int action, int mods) {
	m_player.mouseReleaseEvent(button, action, mods);
}

void GameWindow::mouseMoveEvent(float xpos, float ypos) {
    m_player.mouseMoveEvent(xpos, ypos);
}

void GameWindow::resizeGL(int w, int h) {

	glViewport(0, 0, w, h);
    m_camera.changeViewportSize(w, h);
    m_framebuffer.changeDimension(this, w, h);
}

float GameWindow::getFPS() const {
    float fps = 0;
    for (int i = 0; i < FPS_FRAME_NUMBER; ++i) {
        fps += m_lastDeltas[i];
    }
    return 1000.0f / (fps / (float)FPS_FRAME_NUMBER);
}

ChunkManager& GameWindow::getChunkManager() {
    return m_chunkManager;
}

PhysicManager& GameWindow::getPhysicManager() {
    return m_physicManager;
}

FrameBuffer& GameWindow::getFrameBuffer() {
    return m_framebuffer;
}

int GameWindow::width() {
	int w, h;
	glfwGetFramebufferSize(m_window, &w, &h);
	return w;
}

int GameWindow::height() {
	int w, h;
	glfwGetFramebufferSize(m_window, &w, &h);
	return h;
}
