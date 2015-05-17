#pragma once

#include <glad/glad.h>
#include "camera.h"
#include "player.h"

#include "utilities/time.h"

#include "chunkmanager.h"
#include "physic/physicmanager.h"
#include "npc/npcmanager.h"
#include "postprocess/framebuffer.h"
#include "postprocess/postprocess.h"

#include "npc/npc.h"

class OpenGLTexture;
class GLFWwindow;

// Le nombre de frames à considérer pour le calcul des FPS
#define FPS_FRAME_NUMBER 100

class GameWindow {
public:
    /**
     * @brief Constructeur de la classe GameWindow.
     */
	GameWindow(int worldSeed);
    ~GameWindow();
    /**
     * @brief Initialise la fenêtre.
     */
    void init();
	/**
	* @brief Lance la boucle de rendu
	*/
	void run();
    /**
     * @brief Renvoie le delta de la frame.
     */
    int getDeltaTime();

	void keyPressEvent(int key, int scancode, int action, int mods);
	void keyReleaseEvent(int key, int scancode, int action, int mods);
	void mousePressEvent(int button, int action, int mods, float xpos, float ypos);
	void mouseReleaseEvent(int button, int action, int mods);
	void mouseMoveEvent(float xpos, float ypos);
    void resizeGL(int w, int h);

    /**
     * @brief Renvoie la camera actuelle de la fenêtre.
     */
    Camera& getCamera();
    /**
     * @brief Renvoie le nombre de frames par seconde.
     */
    float getFPS() const;

    ChunkManager& getChunkManager();
    PhysicManager& getPhysicManager();
    FrameBuffer& getFrameBuffer();

	int width();
	int height();

private:
	/**
	* @brief Redessine la fenêtre.
	*/
	void render();

	GLFWwindow* m_window;

    Camera m_camera;
    Player m_player;

    int m_lastDelta;
    int m_currentDeltaIndex;
    Time m_deltaTimer;
    int m_lastDeltas[FPS_FRAME_NUMBER];
    bool m_isInitialized;

    ChunkManager m_chunkManager;
    PhysicManager m_physicManager;
    NpcManager m_npcManager;

    FrameBuffer m_framebuffer;
    PostProcess m_waterPostProcess;

    bool m_hasPhysic;
};
