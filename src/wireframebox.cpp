#include "wireframebox.h"

#include "gamewindow.h"
#include "utilities/openglbuffer.h"
#include "utilities/openglprogramshader.h"
#include "utilities/openglvertexarrayobject.h"

#include <iostream>

#define GLM_FORCE_PURE
#include "glm/gtc/type_ptr.hpp"

WireframeBox::WireframeBox() : m_position{0.0f, 0.0f, 0.0f}, m_color{0.0f, 0.0f, 1.0f},
                                m_width(CHUNK_SCALE), m_height(CHUNK_SCALE) {

}

WireframeBox::~WireframeBox() {
    destroy( nullptr );
}

void WireframeBox::init(GameWindow* gl) {
    m_program = std::make_unique<OpenglProgramShader>(gl);
    m_program->addShaderFromSourceFile(OpenGLShaderType::Vertex, "shaders/wireframeBox.vs");
    m_program->addShaderFromSourceFile(OpenGLShaderType::Fragment, "shaders/wireframeBox.ps");
    m_program->link();

    m_matrixUniform = m_program->uniformLocation("viewProj");
    m_posUniform = m_program->uniformLocation("boxPosition");
    m_colorUniform = m_program->uniformLocation("color");

    m_program->bind();
    m_program->setUniformValue("boxWidth", m_width);
    m_program->setUniformValue("boxHeight", m_height);
    m_program->release();

    // Les indices des sommets à afficher (Les sommets sont calculés dans le shader)
    GLushort indices[24] = {
        0, 1,
        0, 2,
        0, 4,
        1, 3,
        1, 5,
        2, 3,
        2, 6,
        3, 7,
        4, 5,
        4, 6,
        5, 7,
        6, 7
    };

    m_indices = std::make_unique<OpenGLBuffer>(gl, OpenGLBuffer::IndexBuffer);
    m_indices->create();

    m_vao = std::make_unique<OpenGLVertexArrayObject>(gl);
    m_vao->create();
    m_vao->bind();
    m_indices->bind();
    m_indices->setUsagePattern(OpenGLBuffer::StaticDraw);
    m_indices->allocate(indices, sizeof(indices));
    m_vao->release();

}

void WireframeBox::destroy(GameWindow* /*gl*/) {
    m_program = nullptr;
    m_indices = nullptr;
    m_vao = nullptr;
}

void WireframeBox::draw(GameWindow* gl) {
    m_program->bind();
    gl->glUniformMatrix4fv(m_matrixUniform, 1, GL_FALSE, glm::value_ptr(gl->getCamera().getViewProjMatrix()));
    gl->glUniform3fv(m_posUniform, 1, glm::value_ptr(m_position));
    gl->glUniform3fv(m_colorUniform, 1, glm::value_ptr(m_color));
    //m_program->setUniformValue(m_matrixUniform, glm::value_ptr(gl->getCamera().getViewProjMatrix()));
    //m_program->setUniformValue(m_posUniform, m_position);
    //m_program->setUniformValue(m_colorUniform, m_color);

    m_vao->bind();

    gl->glDrawElements(GL_LINES, 24, GL_UNSIGNED_SHORT, nullptr);

    m_vao->release();
    m_program->release();
}

void WireframeBox::setPosition(const glm::vec3& position) {
    m_position = position;
}

void WireframeBox::setColor(float r, float g, float b) {
    m_color = glm::vec3(r, g, b);
}

void WireframeBox::setSize(float width, float height) {
    m_width = width;
    m_height = height;
}

