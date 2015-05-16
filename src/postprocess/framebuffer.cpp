#include "framebuffer.h"

#include "../gamewindow.h"

#include "utilities/openglprogramshader.h"
#include "utilities/openglvertexarrayobject.h"

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
    m_program = std::make_unique<OpenglProgramShader>();
    m_program->addShaderFromSourceCode(OpenGLShaderType::Vertex, vertexShaderSource);
    m_program->addShaderFromSourceCode(OpenGLShaderType::Fragment, fragmentShaderSource);
    m_program->link();
    m_program->bind();
    m_program->setUniformValue("texture", 0);
    m_program->release();

    // On crée le FrameBuffer
    glGenFramebuffers(1, &m_framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);

    // On crée la texture contenant la couleur
    glGenTextures(2, m_colorTexture);
    for (int i = 0; i < 2; ++i) {
        glBindTexture(GL_TEXTURE_2D, m_colorTexture[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, gl->size().width(), gl->size().height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    }

    // On crée la texture contenant la profondeur
    glGenTextures(1, &m_depthTexture);
    glBindTexture(GL_TEXTURE_2D, m_depthTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, gl->size().width(), gl->size().height(), 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

    m_initialized = true;
    m_vao = std::make_unique<OpenGLVertexArrayObject>();
    m_vao->create();
}

void FrameBuffer::destroy(GameWindow* /*gl*/) {
    m_program = nullptr;
    if (m_initialized) {
        glDeleteTextures(2, m_colorTexture);
        glDeleteTextures(1, &m_depthTexture);
        glDeleteFramebuffers(1, &m_framebuffer);
        m_vao->destroy();
        m_vao = nullptr;
        m_initialized = false;
    }
}

void FrameBuffer::changeDimension(GameWindow* /*gl*/, int width, int height) {
    if (m_initialized) {
        // On re-crée les textures à la bonne taille

        for (int i = 0; i < 2; ++i) {
            glBindTexture(GL_TEXTURE_2D, m_colorTexture[i]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
        }

        glBindTexture(GL_TEXTURE_2D, m_depthTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

void FrameBuffer::begin(GameWindow* /*gl*/) {
    if (m_initialized) {
        m_colorTextureIndex = 0;
        glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);

        // On attache les textures au framebuffer
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthTexture, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colorTexture[m_colorTextureIndex], 0);

        GLenum drawBuffers[1] = {GL_COLOR_ATTACHMENT0};
        glDrawBuffers(1, drawBuffers);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
}

void FrameBuffer::end(GameWindow* /*gl*/) {
    if (m_initialized) {
        // On désactive le framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        // On désactive le test de profondeur
        glDisable(GL_DEPTH_TEST);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_colorTexture[m_colorTextureIndex]);

        // Rendu
        m_program->bind();
        m_vao->bind();

        glDrawArrays(GL_TRIANGLES, 0, 6);

        // On remet les états à 0
        m_vao->release();
        m_program->release();

        glEnable(GL_DEPTH_TEST);
    }
}

GLuint FrameBuffer::getColorTexture() const {
    return m_colorTexture[(m_colorTextureIndex + 1) % 2];
}

GLuint FrameBuffer::getDepthTexture() const {
    return m_depthTexture;
}

void FrameBuffer::switchColorTexture(GameWindow* /*gl*/) {
    if (m_initialized) {
        m_colorTextureIndex = (m_colorTextureIndex + 1) % 2;
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colorTexture[m_colorTextureIndex], 0);
    }
}
