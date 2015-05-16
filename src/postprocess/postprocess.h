#pragma once

#include <glad/glad.h>
#include "utilities/time.h"
#include <string>
#include <memory>

class GameWindow;
class OpenglProgramShader;
class OpenGLVertexArrayObject;
class QString;

/**
 * @brief Classe permettant d'afficher un effet de post-process sur tout l'écran.
 */
class PostProcess {
public:
    PostProcess(const std::string& fragmentShader);
    ~PostProcess();

    void init(GameWindow* gl);
    void destroy(GameWindow* gl);

    void render(GameWindow* gl);

private:
    // Le nom du fragment shader à utiliser
    std::string m_fragmentShader;
    // Le shader affichant l'effet
    std::unique_ptr<OpenglProgramShader> m_program;
    // Le vertex array object
    std::unique_ptr<OpenGLVertexArrayObject> m_vao;

    Time m_time;
    int m_timeUniformLocation;
};
