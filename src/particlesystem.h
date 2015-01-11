#pragma once

#include <QtGui/QVector3D>
#include <QTime>
#include "chunk.h"

class GameWindow;
class QOpenGLTexture;
class QOpenGLShaderProgram;
class QOpenGLBuffer;
class QOpenGLVertexArrayObject;

/**
 * @brief Classe permettant d'afficher des particules.
 */
class ParticleSystem {
public:
    /**
     * @param count Le nombre de particules à afficher.
     * @param lifetime La durée de vie des particules (en ms)
     */
    ParticleSystem(int count, int lifetime);
    ~ParticleSystem();

    void init(GameWindow* gl);
    void destroy(GameWindow* gl);
    void update(GameWindow* gl, int dt);
    void draw(GameWindow* gl);

    /**
     * @brief Modifie la position sur à laquelle les particules sont émises.
     * @param position La nouvelle position.
     */
    void setSpawnPosition(const QVector3D& position);
    /**
     * @brief Modifie le type de voxel à afficher sur les particules.
     */
    void setVoxelType(VoxelType type);
    /**
     * @brief Envoie une salve de particules (Les particules précédantes disparaissent).
     */
    void spawn();
    /**
     * @brief Les dernières particules émisent sont-elles mortent ?
     */
    bool isOver();

private:
    // Le shader affichant les particules
    QOpenGLShaderProgram* m_program;
    // L'atlas de textures
    QOpenGLTexture* m_atlas;
    // Le buffer contenant la position des particules
    QOpenGLBuffer* m_vertices;
    QOpenGLVertexArrayObject* m_vao;
    int m_matrixUniform;
    int m_typeUniform;

    // Le nombre de particules
    int m_count;
    // La durée de vie des particules
    int m_lifeTime;
    // Timer calculant le temps depuis la création des dernières particules
    QTime m_spawnTime;
    // Le type de particules à afficher
    VoxelType m_voxelType;
    // La position de départ des particules
    QVector3D m_spawnPosition;

    // Tableau des positions des particules
    QVector3D* m_positions;
    // Tableau des vitesses des particules
    QVector3D* m_velocities;
};
