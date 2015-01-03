#pragma once

#include <QtGui/QOpenGLFunctions>

#include "chunk.h"
#include "chunkmanager.h"


struct Buffer;
class ChunkManager;

class MeshGenerator
{
public:
	MeshGenerator(ChunkManager* manager);
    ~MeshGenerator();

    /**
     * @brief Génére un mesh à partir d'une grille de voxels.
     * @param data La grille à partir de laquelle on génére le mesh.
     * @param vertex Les vertices du mesh.
     * @return Le nombre de vertices dans le mesh.
     */
	int generate(Voxel* data, Coords chunkPos, Buffer* buffer, GLuint* vertices, bool waterPass = false);

private:
    Voxel getVoxel(Voxel* data, int i, int j, int k);
    void setVoxel(Voxel* data, int i, int j, int k, Voxel voxel);
	GLuint getVertex(int x, int y, int z, int normalIndex, TextureID tex, uint8 light);
    Voxel* m_mask;
	uint8* m_light;
    bool* m_offsetNormal;
    Voxel* m_waterPassGrid;

	ChunkManager* m_ChunkManager;
};
