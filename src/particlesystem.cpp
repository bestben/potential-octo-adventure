#include "particlesystem.h"

#include "gamewindow.h"

#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QOpenGLBuffer>
#include <QtGui/QOpenGLTexture>
#include <QtGui/QOpenGLVertexArrayObject>

ParticleSystem::ParticleSystem(int count, int lifetime) : m_count{count}, m_lifeTime{lifetime} {
    m_positions = new QVector3D[m_count];
    m_velocities = new QVector3D[m_count];
}

ParticleSystem::~ParticleSystem() {
    delete[] m_positions;
    delete[] m_velocities;
}


void ParticleSystem::init(GameWindow* gl) {
    gl->glPointSize(20.0f);

    m_program = new QOpenGLShaderProgram(gl);
    m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, QString(":/particle.vs"));
    m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, QString(":/particle.ps"));
    m_program->link();

    m_matrixUniform = m_program->uniformLocation("mvp");
    m_typeUniform = m_program->uniformLocation("voxelType");
    GLuint verticesIndex = m_program->attributeLocation("position");

    m_program->bind();
    m_program->setUniformValue("atlas", 0);
    m_program->release();

    QImage image(":/atlas.png");
    m_atlas = new QOpenGLTexture(image);
    m_atlas->setMagnificationFilter(QOpenGLTexture::Nearest);

    m_vertices = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    m_vertices->create();

    m_vao = new QOpenGLVertexArrayObject(gl);
    m_vao->create();
    m_vao->bind();
    m_vertices->bind();
    m_vertices->setUsagePattern(QOpenGLBuffer::DynamicDraw);
    m_vertices->allocate(nullptr, sizeof(QVector3D) * m_count);
    m_program->enableAttributeArray(verticesIndex);
    gl->glVertexAttribPointer(verticesIndex, 3, GL_FLOAT, false, 0, 0);

    m_vao->release();
    m_spawnTime.start();
}

void ParticleSystem::destroy(GameWindow* gl) {
    delete m_program;
    delete m_vao;
    delete m_vertices;
    delete m_atlas;
}

void ParticleSystem::update(GameWindow* gl, int dt) {
    if (m_spawnTime.elapsed() < m_lifeTime) {
        int count = m_count;
        QVector3D g(0.0f, 9.81f, 0.0f);
        float delta = (float)dt / 1000.0f;

        for (int i = 0; i < count; ++i) {
            m_velocities[i] -= 2.0f * g * delta;
            m_positions[i] += m_velocities[i] * delta;
        }
        m_vertices->bind();
        m_vertices->write(0, m_positions, sizeof(QVector3D) * m_count);
    }
}

void ParticleSystem::draw(GameWindow* gl) {
    if (m_spawnTime.elapsed() < m_lifeTime) {
        m_program->bind();
        m_vao->bind();

        m_atlas->bind(0);

        m_program->setUniformValue(m_typeUniform, (int)getTexture(m_voxelType, 0));
        m_program->setUniformValue(m_matrixUniform, gl->getCamera().getViewProjMatrix());

        gl->glDrawArrays(GL_POINTS, 0, m_count);

        m_vao->release();
        m_program->release();
    }
}

void ParticleSystem::setSpawnPosition(const QVector3D& position) {
    m_spawnPosition = position;
}

void ParticleSystem::setVoxelType(VoxelType type) {
    m_voxelType = type;
}

void ParticleSystem::spawn() {
    int count = m_count;
    QVector3D spawnPos = m_spawnPosition;
    for (int i = 0; i < count; ++i) {
        float vx = ((float)rand() / (float)RAND_MAX) * 2.0 - 1.0;
        float vz = ((float)rand() / (float)RAND_MAX) * 2.0 - 1.0;
        m_velocities[i] = QVector3D(vx, 1.0f, vz) * 10.0f;
        m_positions[i] = spawnPos;
    }
    m_spawnTime.start();
}

bool ParticleSystem::isOver() {
    return m_spawnTime.elapsed() > m_lifeTime;
}
