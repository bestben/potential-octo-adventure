#pragma once

#include "glm/vec3.hpp"
#include <memory>

class GameWindow;
class OpenglProgramShader;
class OpenGLVertexArrayObject;
class OpenGLBuffer;
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
    void setPosition(const glm::vec3& position);
    void setSize(float width, float height);

    /**
     * @brief Modifie le taux de dégat du voxel.
     * @param damage La quantité de dégat dans l'intervalle [0.0, 1.0].
     */
    void setDamage(float damage);

private:
    // Le shader affichant le voxel
    std::unique_ptr<OpenglProgramShader> m_program;
    // L'atlas de textures
    QOpenGLTexture* m_atlas;
    // Les vertices du voxel
    std::unique_ptr<OpenGLBuffer> m_vertices;
    std::unique_ptr<OpenGLBuffer> m_normals;
    std::unique_ptr<OpenGLVertexArrayObject> m_vao;

    int m_posUniform;
    int m_damageUniform;
    int m_matrixUniform;

    // La position du voxel à afficher
    glm::vec3 m_position;
    // La largeur du voxel à afficher
    float m_width;
    // La hauteur du voxel à afficher
    float m_height;

    // Le taux de dommage entre [0.0, 1.0]
    float m_damage;
};
