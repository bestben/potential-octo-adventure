#pragma once

#include <QtGui/QOpenGLFunctions>
#include <memory>

class GameWindow;
class OpenglProgramShader;
class QOpenGLVertexArrayObject;

/**
 * Classe Framebuffer permettant d'avoir accès à un framebuffer perso.
 * Permet d'appliquer des effets sur tout l'écran.
 */
class FrameBuffer {
public:
    FrameBuffer();
    ~FrameBuffer();

    /**
     * @brief Initialise le framebuffer.
     */
    void initialize(GameWindow* gl);
    /**
     * @brief Détruit le framebuffer.
     */
    void destroy(GameWindow* gl);
    /**
     * @brief Change la dimmension du framebuffer.
     * @param width La nouvelle largeur du framebuffer
     * @param height La nouvelle hauteur du framebuffer
     */
    void changeDimension(GameWindow* gl, int width, int height);
    /**
     * @brief Prépare le framebuffer pour le rendu.
     * À appeler avant le rendu de la scene.
     */
    void begin(GameWindow* gl);
    /**
     * @brief Termine le rendu de la scene.
     * À appeler après le rendu de la scene.
     */
    void end(GameWindow* gl);
    /**
     * @brief Renvoie l'identifiant de la texture contenant le résultat du rendu.
     */
    GLuint getColorTexture() const;
    /**
     * @brief Renvoie l'identifiant de la texture contenant la profondeur du rendu.
     */
    GLuint getDepthTexture() const;
    /**
     * @brief Change la texture dans laquelle l'on fait le rendu.
     */
    void switchColorTexture(GameWindow* gl);

private:
    // Permet de savoir si le framebuffer est initialisé
    bool m_initialized;
    // Le shader utilisé pour dessiner le framebuffer
    std::unique_ptr<OpenglProgramShader> m_program;
    // Le vertex array object
    QOpenGLVertexArrayObject* m_vao;
    // L'identifiant de framebuffer
    GLuint m_framebuffer;
    // L'identifiant de la texture de couleur
    GLuint m_colorTexture[2];
    // L'identifiant de la texture de profondeur
    GLuint m_depthTexture;
    // L'index de la texture actuelle
    int m_colorTextureIndex;

};
