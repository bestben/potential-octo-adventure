#pragma once

#include "glm/vec3.hpp"
#include <memory>

class GameWindow;
class OpenglProgramShader;
class QOpenGLVertexArrayObject;
class QOpenGLBuffer;

/**
 * @brief Classe permettant d'afficher les contours d'une boite.
 */
class WireframeBox {
public:
    WireframeBox();
    ~WireframeBox();

    void init(GameWindow* gl);
    void destroy(GameWindow* gl);
    void draw(GameWindow* gl);

    /**
     * @brief Modifie la position de la boite.
     */
    void setPosition(const glm::vec3& position);
    /**
     * @brief Modifie la couleur de la boite.
     */
    void setColor(float r, float g, float b);
    /**
     * @brief Modifie la taille de la boite.
     */
    void setSize(float width, float height);

private:
    // Le shader affichant la boite
    std::unique_ptr<OpenglProgramShader> m_program;
    // Les indices des sommets de la boite
    QOpenGLBuffer* m_indices;
    QOpenGLVertexArrayObject* m_vao;

    int m_posUniform;
    int m_colorUniform;
    int m_matrixUniform;

    glm::vec3 m_position;
    glm::vec3 m_color;
    float m_width;
    float m_height;
};
