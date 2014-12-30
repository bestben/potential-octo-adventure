#pragma once

#include <QtGui/QOpenGLFunctions>

#include "chunk.h"

struct Buffer;

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
    int generate(Voxel* data, Buffer* buffer, GLuint* vertices, bool waterPass = false);

private:
    Voxel getVoxel(Voxel* data, int i, int j, int k);
    void setVoxel(Voxel* data, int i, int j, int k, Voxel voxel);
	GLuint getVertex(int x, int y, int z, int normalIndex, TextureID tex, Voxel voxel);

    Voxel* m_mask;
    bool* m_offsetNormal;
    Voxel* m_waterPassGrid;
};
