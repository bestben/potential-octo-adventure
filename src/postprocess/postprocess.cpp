#include "postprocess.h"

#include "../gamewindow.h"

#include "utilities/openglprogramshader.h"
#include "utilities/openglvertexarrayobject.h"

PostProcess::PostProcess(const std::string& fragmentShader) :
    m_program{nullptr},
    m_vao{nullptr},
    m_time{},
    m_timeUniformLocation{0}
{
    m_fragmentShader = fragmentShader;
}

PostProcess::~PostProcess() {

}

void PostProcess::init(GameWindow* gl) {
    m_program = std::make_unique<OpenglProgramShader>(gl);

    m_program->addShaderFromSourceFile(OpenGLShaderType::Vertex, "shaders/postProcess.vs");
    m_program->addShaderFromSourceFile(OpenGLShaderType::Fragment, m_fragmentShader);
    m_program->link();
    m_timeUniformLocation = m_program->uniformLocation("time");
    m_program->bind();
    m_program->setUniformValue("texture", 0);
    m_program->release();

    m_vao = std::make_unique<OpenGLVertexArrayObject>(gl);
    m_vao->create();

    m_time.start();
}

void PostProcess::destroy(GameWindow* /*gl*/) {
    m_vao = nullptr;
    m_program = nullptr;
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
