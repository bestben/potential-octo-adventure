#include "postprocess.h"

#include "../gamewindow.h"

#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QOpenGLVertexArrayObject>

PostProcess::PostProcess(const QString& fragmentShader) : m_fragmentShader{fragmentShader} {

}

PostProcess::~PostProcess() {

}

void PostProcess::init(GameWindow* gl) {
    m_program = new QOpenGLShaderProgram(gl);

    m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, QString(":/postProcess.vs"));
    m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, m_fragmentShader);
    m_program->link();
    m_timeUniformLocation = m_program->uniformLocation("time");
    m_program->bind();
    m_program->setUniformValue("texture", 0);
    m_program->release();

    m_vao = new QOpenGLVertexArrayObject(gl);
    m_vao->create();

    m_time.start();
}

void PostProcess::destroy(GameWindow* gl) {
    delete m_program;
    delete m_vao;
}

void PostProcess::render(GameWindow* gl) {
    FrameBuffer& fb = gl->getFrameBuffer();

    fb.switchColorTexture(gl);
    gl->glActiveTexture(GL_TEXTURE0);
    gl->glBindTexture(GL_TEXTURE_2D, fb.getColorTexture());

    m_program->bind();
    m_vao->bind();

    if (m_timeUniformLocation != -1) {
        m_program->setUniformValue(m_timeUniformLocation, (float)m_time.elapsed() / 1000.0f);
    }

    gl->glDrawArrays(GL_TRIANGLES, 0, 6);

    m_vao->release();
    m_program->release();
}
