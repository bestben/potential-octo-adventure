#include "voxelbuffer.h"

#include "gamewindow.h"
#include <QtGui/QOpenGLBuffer>
#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QOpenGLVertexArrayObject>
#include <QtGui/QOpenGLTexture>
#include <QImage>

#include <iostream>

VoxelBuffer::VoxelBuffer() : m_position{0.0f, 0.0f, 0.0f}, m_width(CHUNK_SCALE), m_height(CHUNK_SCALE) {

}

VoxelBuffer::~VoxelBuffer() {

}

void VoxelBuffer::init(GameWindow* gl) {
    m_program = new QOpenGLShaderProgram(gl);
    m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, QString(":/voxelBuffer.vs"));
    m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, QString(":/voxelBuffer.ps"));
    m_program->link();

    m_matrixUniform = m_program->uniformLocation("viewProj");
    m_posUniform = m_program->uniformLocation("boxPosition");
    m_damageUniform = m_program->uniformLocation("damage");
    GLuint normalIndex = m_program->attributeLocation("normalIndex");
    GLuint verticesIndex = m_program->attributeLocation("vertexIndex");

    m_program->bind();
    m_program->setUniformValue("atlas", 0);
    m_program->setUniformValue("boxWidth", m_width);
    m_program->setUniformValue("boxHeight", m_height);

    m_program->release();

    QImage image(":/atlas.png");
    QColor color(127, 127, 127);
    //m_atlas = new QOpenGLTexture(image.createMaskFromColor(color.rgb()));
    m_atlas = new QOpenGLTexture(image);
    m_atlas->setMagnificationFilter(QOpenGLTexture::Nearest);

    GLint vertices[36] = {
        3, 1, 0, 0, 2, 3,
        7, 6, 4, 4, 5, 7,
        5, 4, 0, 0, 1, 5,
        4, 6, 2, 2, 0, 4,
        7, 5, 1, 1, 3, 7,
        6, 7, 3, 3, 2, 6
    };


    GLint normals[36] = {
        0, 0, 0, 0, 0, 0,
        1, 1, 1, 1, 1, 1,

        2, 2, 2, 2, 2, 2,
        4, 4, 4, 4, 4, 4,

        5, 5, 5, 5, 5, 5,
        3, 3, 3, 3, 3, 3
    };

    m_vertices = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    m_vertices->create();

    m_normals = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    m_normals->create();


    m_vao = new QOpenGLVertexArrayObject(gl);
    m_vao->create();
    m_vao->bind();
    m_vertices->bind();
    m_vertices->setUsagePattern(QOpenGLBuffer::StaticDraw);
    m_vertices->allocate(vertices, sizeof(vertices));
    m_program->enableAttributeArray(verticesIndex);
    gl->glVertexAttribIPointer(verticesIndex, 1, GL_INT, 0, 0);

    m_normals->bind();
    m_normals->setUsagePattern(QOpenGLBuffer::StaticDraw);
    m_normals->allocate(normals, sizeof(normals));
    m_program->enableAttributeArray(normalIndex);
    gl->glVertexAttribIPointer(normalIndex, 1, GL_INT, 0, 0);

    m_vao->release();

}

void VoxelBuffer::destroy(GameWindow* gl) {
    delete m_program;
    delete m_vertices;
    delete m_vao;
}

void VoxelBuffer::setDamage(float damage) {
    m_damage = damage;
}

void VoxelBuffer::draw(GameWindow* gl) {
    m_program->bind();
    m_program->setUniformValue(m_matrixUniform, gl->getCamera().getViewProjMatrix());
    m_program->setUniformValue(m_posUniform, m_position);
    m_program->setUniformValue(m_damageUniform, m_damage);

    m_atlas->bind(0);

    m_vao->bind();

    gl->glDrawArrays(GL_TRIANGLES, 0, 36);

    m_vao->release();
    m_program->release();
}

void VoxelBuffer::setPosition(const QVector3D& position) {
    m_position = position;
}

void VoxelBuffer::setSize(float width, float height) {
    m_width = width;
    m_height = height;
}

