#pragma once

#include <QtGui/QVector3D>
#include <QTime>
#include "chunk.h"

class GameWindow;
class QOpenGLTexture;
class QOpenGLShaderProgram;
class QOpenGLBuffer;
class QOpenGLVertexArrayObject;

class ParticleSystem {
public:
    ParticleSystem(int count, int lifetime);
    ~ParticleSystem();

    void init(GameWindow* gl);
    void destroy(GameWindow* gl);
    void update(GameWindow* gl, int dt);
    void draw(GameWindow* gl);

    void setSpawnPosition(const QVector3D& position);
    void setVoxelType(VoxelType type);
    void spawn();

    bool isOver();

private:
    QOpenGLShaderProgram* m_program;
    // L'atlas de textures
    QOpenGLTexture* m_atlas;
    QOpenGLBuffer* m_vertices;
    QOpenGLVertexArrayObject* m_vao;

    int m_matrixUniform;
    int m_typeUniform;

    int m_count;
    int m_lifeTime;
    QTime m_spawnTime;
    VoxelType m_voxelType;
    QVector3D m_spawnPosition;

    QVector3D* m_positions;
    QVector3D* m_velocities;
};
