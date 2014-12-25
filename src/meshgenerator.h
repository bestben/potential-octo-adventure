#pragma once

#include <QtGui/QOpenGLFunctions>


#include "chunk.h"

class MeshGenerator
{
public:
    MeshGenerator();
    ~MeshGenerator();

    /**
     * @brief Génére un mesh à partir d'une grille de voxels.
     * @param data La grille à partir de laquelle on génére le mesh.
     * @param vertex Les vertices du mesh.
     * @return Le nombre de vertices dans le mesh.
     */
    int generate(Voxel* data, GLuint* vertices);

private:
    GLuint getVoxel(Voxel* data, int i, int j, int k);
    GLuint getVertex(int x, int y, int z, int normalIndex, int voxel);

    int* m_mask;
};
