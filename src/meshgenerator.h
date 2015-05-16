#pragma once

#include "chunk.h"
#include "chunkmanager.h"


struct Buffer;
class ChunkManager;

/**
 * @brief Classe permettant de générer un mesh à partir d'une grille de voxels.
 */
class MeshGenerator {
public:
	MeshGenerator(ChunkManager* manager);
    ~MeshGenerator();

    /**
     * @brief Génére un mesh à partir d'une grille de voxels.
     * @param data La grille à partir de laquelle on génére le mesh.
     * @param vertex Les vertices du mesh.
     * @param waterPass Si l'on se trouve dans la phase générant les voxels d'eau.
     * @return Le nombre de vertices dans le mesh.
     */
	int generate(Voxel* data, Coords chunkPos, Buffer* buffer, GLuint* vertices, bool waterPass = false);

private:
    /*
     * Renvoie un voxel dans une grille.
     */
    Voxel getVoxel(Voxel* data, int i, int j, int k);
    /*
     * Modifie un voxel dans une grille.
     */
    void setVoxel(Voxel* data, int i, int j, int k, Voxel voxel);
    /**
     * Encode un vertex dans un entier.
     */
	GLuint getVertex(int x, int y, int z, int normalIndex, TextureID tex, uint8 light);

    Voxel* m_mask;
	uint8* m_light;
    bool* m_offsetNormal;
    // Grille stokant les voxels d'eau entre la première et la seconde phase
    Voxel* m_waterPassGrid;

	ChunkManager* m_ChunkManager;
};
