#pragma once

#include <QtGui/QVector3D>

class GameWindow;
class QOpenGLShaderProgram;
class QOpenGLVertexArrayObject;
class QOpenGLBuffer;
class QOpenGLTexture;

/**
 * Classe permettant d'afficher un unique voxel.
 */
class VoxelBuffer {
public:
    VoxelBuffer();
    ~VoxelBuffer();

    void init(GameWindow* gl);
    void destroy(GameWindow* gl);
    void draw(GameWindow* gl);

    void setPosition(const QVector3D& position);
    void setSize(float width, float height);

    void setDamage(float damage);

private:
    QOpenGLShaderProgram* m_program;
    // L'atlas de textures
    QOpenGLTexture* m_atlas;
    QOpenGLBuffer* m_vertices;
    QOpenGLBuffer* m_normals;
    QOpenGLVertexArrayObject* m_vao;

    int m_posUniform;
    int m_damageUniform;
    int m_matrixUniform;

    QVector3D m_position;
    float m_width;
    float m_height;

    float m_damage;
};
