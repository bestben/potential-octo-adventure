#pragma once

#include <QtGui/QVector3D>

class GameWindow;
class QOpenGLShaderProgram;
class QOpenGLVertexArrayObject;
class QOpenGLBuffer;
class QOpenGLTexture;

/**
 * Classe permettant d'afficher un unique voxel avec une texture endommagée.
 */
class VoxelBuffer {
public:
    VoxelBuffer();
    ~VoxelBuffer();

    void init(GameWindow* gl);
    void destroy(GameWindow* gl);
    void draw(GameWindow* gl);

    /**
     * @brief Modifie la position d'affichage du cube.
     * @param position La nouvelle position.
     */
    void setPosition(const QVector3D& position);
    void setSize(float width, float height);

    /**
     * @brief Modifie le taux de dégat du voxel.
     * @param damage La quantité de dégat dans l'intervalle [0.0, 1.0].
     */
    void setDamage(float damage);

private:
    // Le shader affichant le voxel
    QOpenGLShaderProgram* m_program;
    // L'atlas de textures
    QOpenGLTexture* m_atlas;
    // Les vertices du voxel
    QOpenGLBuffer* m_vertices;
    QOpenGLBuffer* m_normals;
    QOpenGLVertexArrayObject* m_vao;

    int m_posUniform;
    int m_damageUniform;
    int m_matrixUniform;

    // La position du voxel à afficher
    QVector3D m_position;
    // La largeur du voxel à afficher
    float m_width;
    // La hauteur du voxel à afficher
    float m_height;

    // Le taux de dommage entre [0.0, 1.0]
    float m_damage;
};
