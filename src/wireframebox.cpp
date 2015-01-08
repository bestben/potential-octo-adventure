#include "wireframebox.h"

#include "gamewindow.h"
#include <QtGui/QOpenGLBuffer>
#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QOpenGLVertexArrayObject>

#include <iostream>

WireframeBox::WireframeBox() : m_position{0.0f, 0.0f, 0.0f}, m_color{0.0f, 0.0f, 1.0f}, m_width(CHUNK_SCALE), m_height(CHUNK_SCALE) {

}

WireframeBox::~WireframeBox() {

}

void WireframeBox::init(GameWindow* gl) {
    m_program = new QOpenGLShaderProgram(gl);
    m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, QString(":/wireframeBox.vs"));
    m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, QString(":/wireframeBox.ps"));
    m_program->link();

    m_matrixUniform = m_program->uniformLocation("viewProj");
    m_posUniform = m_program->uniformLocation("boxPosition");
    m_colorUniform = m_program->uniformLocation("color");

    m_program->bind();
    m_program->setUniformValue("boxWidth", m_width);
    m_program->setUniformValue("boxHeight", m_height);
    m_program->release();

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

    m_indices = new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
    m_indices->create();

    m_vao = new QOpenGLVertexArrayObject(gl);
    m_vao->create();
    m_vao->bind();
    m_indices->bind();
    m_indices->setUsagePattern(QOpenGLBuffer::StaticDraw);
    m_indices->allocate(indices, sizeof(indices));
    m_vao->release();

}

void WireframeBox::destroy(GameWindow* gl) {
    delete m_program;
    delete m_indices;
    delete m_vao;
}

void WireframeBox::draw(GameWindow* gl) {
    m_program->bind();
    m_program->setUniformValue(m_matrixUniform, gl->getCamera().getViewProjMatrix());
    m_program->setUniformValue(m_posUniform, m_position);
    m_program->setUniformValue(m_colorUniform, m_color);

    m_vao->bind();

    gl->glDrawElements(GL_LINES, 24, GL_UNSIGNED_SHORT, nullptr);

    m_vao->release();
    m_program->release();
}

void WireframeBox::setPosition(const QVector3D& position) {
    m_position = position;
}

void WireframeBox::setColor(float r, float g, float b) {
    m_color = QVector3D(r, g, b);
}

void WireframeBox::setSize(float width, float height) {
    m_width = width;
    m_height = height;
}

