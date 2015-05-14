#include "framebuffer.h"

#include "../gamewindow.h"

#include "utilities/OpenglProgramShader.h"
#include <QtGui/QOpenGLVertexArrayObject>

static const char* vertexShaderSource =
    "#version 330\n"
    "out vec2 ex_textCoord;\n"
    "void main() {\n"
    "    int id = gl_VertexID;\n"
    "    if (gl_VertexID == 4) {\n"
    "        id = 2;\n"
    "    } else if (gl_VertexID == 5) {\n"
    "        id = 1;\n"
    "    }\n"
    "    int posX = (id >> 1) & 1;\n"
    "    int posY = (id >> 0) & 1;\n"
    "    ex_textCoord = vec2(posX, posY);\n"
    "    gl_Position = vec4(posX * 2.0 - 1.0, posY * 2.0 - 1.0, 0.0, 1.0);\n"
    "}";

static const char* fragmentShaderSource =
    "#version 330\n"
    "out vec4 out_color;\n"
    "in vec2 ex_textCoord;\n"
    "uniform sampler2D texture;\n"
    "void main() {\n"
    "   vec4 col = texture2D( texture, ex_textCoord.xy );\n"
    "   out_color = col;\n"
    "}\n";

FrameBuffer::FrameBuffer() : m_initialized{false}, m_program{nullptr}, m_colorTextureIndex{0} {
}

FrameBuffer::~FrameBuffer() {
}

void FrameBuffer::initialize(GameWindow* gl) {
    // On crée le shader et le vbo pour dessiner le framebuffer à l'écran
    m_program = std::make_unique<OpenglProgramShader>(gl);
    m_program->addShaderFromSourceCode(OpenGLShaderType::Vertex, vertexShaderSource);
    m_program->addShaderFromSourceCode(OpenGLShaderType::Fragment, fragmentShaderSource);
    m_program->link();
    m_program->bind();
    m_program->setUniformValue("texture", 0);
    m_program->release();

    // On crée le FrameBuffer
    gl->glGenFramebuffers(1, &m_framebuffer);
    gl->glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);

    // On crée la texture contenant la couleur
    gl->glGenTextures(2, m_colorTexture);
    for (int i = 0; i < 2; ++i) {
        gl->glBindTexture(GL_TEXTURE_2D, m_colorTexture[i]);
        gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, gl->size().width(), gl->size().height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    }

    // On crée la texture contenant la profondeur
    gl->glGenTextures(1, &m_depthTexture);
    gl->glBindTexture(GL_TEXTURE_2D, m_depthTexture);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
    gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, gl->size().width(), gl->size().height(), 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

    m_initialized = true;
    m_vao = new QOpenGLVertexArrayObject(gl);
    m_vao->create();
}

void FrameBuffer::destroy(GameWindow* gl) {
    if (m_initialized) {
        gl->glDeleteTextures(2, m_colorTexture);
        gl->glDeleteTextures(1, &m_depthTexture);
        gl->glDeleteFramebuffers(1, &m_framebuffer);
        m_vao->destroy();
        delete m_vao;
        m_initialized = false;
    }
}

void FrameBuffer::changeDimension(GameWindow* gl, int width, int height) {
    if (m_initialized) {
        // On re-crée les textures à la bonne taille

        for (int i = 0; i < 2; ++i) {
            gl->glBindTexture(GL_TEXTURE_2D, m_colorTexture[i]);
            gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
        }

        gl->glBindTexture(GL_TEXTURE_2D, m_depthTexture);
        gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
        gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        gl->glBindTexture(GL_TEXTURE_2D, 0);
    }
}

void FrameBuffer::begin(GameWindow* gl) {
    if (m_initialized) {
        m_colorTextureIndex = 0;
        gl->glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);

        // On attache les textures au framebuffer
        gl->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthTexture, 0);
        gl->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colorTexture[m_colorTextureIndex], 0);

        GLenum drawBuffers[1] = {GL_COLOR_ATTACHMENT0};
        gl->glDrawBuffers(1, drawBuffers);

        gl->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
}

void FrameBuffer::end(GameWindow* gl) {
    if (m_initialized) {
        // On désactive le framebuffer
        gl->glBindFramebuffer(GL_FRAMEBUFFER, 0);
        // On désactive le test de profondeur
        gl->glDisable(GL_DEPTH_TEST);

        gl->glActiveTexture(GL_TEXTURE0);
        gl->glBindTexture(GL_TEXTURE_2D, m_colorTexture[m_colorTextureIndex]);

        // Rendu
        m_program->bind();
        m_vao->bind();

        gl->glDrawArrays(GL_TRIANGLES, 0, 6);

        // On remet les états à 0
        m_vao->release();
        m_program->release();

        gl->glEnable(GL_DEPTH_TEST);
    }
}

GLuint FrameBuffer::getColorTexture() const {
    return m_colorTexture[(m_colorTextureIndex + 1) % 2];
}

GLuint FrameBuffer::getDepthTexture() const {
    return m_depthTexture;
}

void FrameBuffer::switchColorTexture(GameWindow* gl) {
    if (m_initialized) {
        m_colorTextureIndex = (m_colorTextureIndex + 1) % 2;
        gl->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colorTexture[m_colorTextureIndex], 0);
    }
}
