#include "particlesystem.h"

#include "gamewindow.h"

#include <glad/glad.h>

#include "utilities/openglprogramshader.h"
#include "utilities/openglbuffer.h"
#include "utilities/opengltexture.h"
#include "utilities/openglvertexarrayobject.h"

#define GLM_FORCE_PURE
#include "glm/gtc/type_ptr.hpp"

ParticleSystem::ParticleSystem(int count, int lifetime) : m_count{count}, m_lifeTime{lifetime} {
    m_positions = new glm::vec3[m_count];
    m_velocities = new glm::vec3[m_count];
}

ParticleSystem::~ParticleSystem() {
    delete[] m_positions;
    delete[] m_velocities;
}


void ParticleSystem::init(GameWindow* /*gl*/) {
    glPointSize(20.0f);

    m_program = std::make_unique<OpenglProgramShader>();
    m_program->addShaderFromSourceFile(OpenGLShaderType::Vertex, "shaders/particle.vs");
    m_program->addShaderFromSourceFile(OpenGLShaderType::Fragment, "shaders/particle.ps");
    m_program->link();

    m_matrixUniform = m_program->uniformLocation("mvp");
    m_typeUniform = m_program->uniformLocation("voxelType");
    GLuint verticesIndex = m_program->attributeLocation("position");

    m_program->bind();
    m_program->setUniformValue("atlas", 0);
    m_program->release();

    m_atlas = std::make_unique<OpenGLTexture>("textures/atlas.png");
    m_atlas->setMagnificationFilter(OpenGLTexture::Nearest);

    m_vertices = std::make_unique<OpenGLBuffer>(OpenGLBuffer::VertexBuffer);
    m_vertices->create();

    m_vao = std::make_unique<OpenGLVertexArrayObject>();
    m_vao->create();
    m_vao->bind();
    m_vertices->bind();
    m_vertices->setUsagePattern(OpenGLBuffer::DynamicDraw);
    m_vertices->allocate(nullptr, sizeof(glm::vec3) * m_count);
    m_program->enableAttributeArray(verticesIndex);
    glVertexAttribPointer(verticesIndex, 3, GL_FLOAT, false, 0, 0);

    m_vao->release();
    m_spawnTime.start();
}

void ParticleSystem::destroy(GameWindow* /*gl*/) {
    m_program = nullptr;
    m_vao = nullptr;
    m_vertices = nullptr;
    m_atlas = nullptr;
}

void ParticleSystem::update(GameWindow* /*gl*/, int dt) {
    if (m_spawnTime.elapsed() < m_lifeTime) {
        int count = m_count;
        glm::vec3 g(0.0f, 9.81f, 0.0f);
        float delta = (float)dt / 1000.0f;

        for (int i = 0; i < count; ++i) {
            m_velocities[i] -= 2.0f * g * delta;
            m_positions[i] += m_velocities[i] * delta;
        }
        m_vertices->bind();
        m_vertices->write(0, m_positions, sizeof(glm::vec3) * m_count);
    }
}

void ParticleSystem::draw(GameWindow* gl) {
    if (m_spawnTime.elapsed() < m_lifeTime) {
        m_program->bind();
        m_vao->bind();

        m_atlas->bind(0);

        m_program->setUniformValue(m_typeUniform, (int)getTexture(m_voxelType, 0));
        glUniformMatrix4fv(m_matrixUniform, 1, GL_FALSE, glm::value_ptr(gl->getCamera().getViewProjMatrix()));
        //m_program->setUniformValue(m_matrixUniform, gl->getCamera().getViewProjMatrix());

        glDrawArrays(GL_POINTS, 0, m_count);

        m_vao->release();
        m_program->release();
    }
}

void ParticleSystem::setSpawnPosition(const glm::vec3& position) {
    m_spawnPosition = position;
}

void ParticleSystem::setVoxelType(VoxelType type) {
    m_voxelType = type;
}

void ParticleSystem::spawn() {
    int count = m_count;
    glm::vec3 spawnPos = m_spawnPosition;
    for (int i = 0; i < count; ++i) {
        float vx = ((float)rand() / (float)RAND_MAX) * 2.0 - 1.0;
        float vz = ((float)rand() / (float)RAND_MAX) * 2.0 - 1.0;
        m_velocities[i] = glm::vec3(vx, 1.0f, vz) * 10.0f;
        m_positions[i] = spawnPos;
    }
    m_spawnTime.start();
}

bool ParticleSystem::isOver() {
    return m_spawnTime.elapsed() > m_lifeTime;
}
