#include "meshgenerator.h"
#include <iostream>

#include "chunkmanager.h"

// TODO(antoine): Meilleur endroit pour cette initialisation ?

VoxelTextureMap VoxelTextures[(uint)VoxelType::COUNT];


inline void initializeTextureMaps(){

	// On ne devrait jamais demander la texture de l'air
	VoxelTextures[(uint)VoxelType::AIR] = { TextureID::ERROR_TEXTURE, TextureID::ERROR_TEXTURE, TextureID::ERROR_TEXTURE, TextureID::ERROR_TEXTURE, TextureID::ERROR_TEXTURE, TextureID::ERROR_TEXTURE};

	VoxelTextures[(uint)VoxelType::GRASS] = {
		TextureID::GRASS,
		TextureID::DIRT,
		TextureID::GRASS_SIDE,
		TextureID::GRASS_SIDE,
		TextureID::GRASS_SIDE,
		TextureID::GRASS_SIDE
	};

	FULL_BLOCK(DIRT)
	FULL_BLOCK(ROCK)
	FULL_BLOCK(SAND)
	FULL_BLOCK(GRAVEL)
	FULL_BLOCK(LAVA)
	FULL_BLOCK(WATER)
	FULL_BLOCK(LEAVES)

	VoxelTextures[(uint)VoxelType::STONE] = {
		TextureID::STONE_TOP,
		TextureID::STONE_TOP,
		TextureID::STONE_SIDE,
		TextureID::STONE_SIDE,
		TextureID::STONE_SIDE,
		TextureID::STONE_SIDE
	};

	VoxelTextures[(uint)VoxelType::TRUNK] = {
		TextureID::TRUNK_TOP,
		TextureID::TRUNK_TOP,
		TextureID::TRUNK_SIDE,
		TextureID::TRUNK_SIDE,
		TextureID::TRUNK_SIDE,
		TextureID::TRUNK_SIDE
	};

}

MeshGenerator::MeshGenerator(ChunkManager* manager) : m_ChunkManager{manager}
{
    m_mask = new Voxel[CHUNK_SIZE * CHUNK_SIZE];
	m_light = new uint8[CHUNK_SIZE * CHUNK_SIZE];

    m_waterPassGrid = new Voxel[CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE];
    m_offsetNormal = new bool[CHUNK_SIZE * CHUNK_SIZE];

	initializeTextureMaps();
}

MeshGenerator::~MeshGenerator() {
	delete[] m_light;
    delete[] m_mask;
    delete[] m_offsetNormal;
    delete[] m_waterPassGrid;
}

int MeshGenerator::generate(Voxel* data, Coords chunkPos, Buffer* buffer, GLuint* vertices, bool waterPass) {
    int vertexCount = 0;
    bool hasWater = false;
    if (!waterPass) {
        memset(m_waterPassGrid, 0, CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE * sizeof(Voxel));
    }
	Voxel emptyVoxel = {};
	emptyVoxel._light = 2;


	Coords offset = chunkPos * CHUNK_SIZE;
    for (int axis = 0; axis < 3; ++axis) {
        const int u = (axis + 1) % 3;
        const int v = (axis + 2) % 3;

        int x[3] = {0}, q[3] = {0};
        memset(m_mask, 0, CHUNK_SIZE * CHUNK_SIZE * sizeof(Voxel));
		memset(m_light, 0, CHUNK_SIZE * CHUNK_SIZE * sizeof(uint8));
        memset(m_offsetNormal, true, CHUNK_SIZE * CHUNK_SIZE * sizeof(bool));

        // Calcule du mask
        q[axis] = 1;
        for (x[axis] = -1; x[axis] < CHUNK_SIZE;) {
            int counter = 0;
            for (x[v] = 0; x[v] < CHUNK_SIZE; ++x[v]) {
                for (x[u] = 0; x[u] < CHUNK_SIZE; ++x[u], ++counter)
                {
					Voxel a, b;

					if (0 <= x[axis])
						a = getVoxel(data, x[0], x[1], x[2]);
					else {
						a = m_ChunkManager->getVoxel(offset + Coords{ x[0], x[1], x[2] });
						//a.type = VoxelType::AIR;
					}
					if (x[axis] < CHUNK_SIZE - 1) {
						b = getVoxel(data, x[0] + q[0], x[1] + q[1], x[2] + q[2]);
					}
					else {
						b = m_ChunkManager->getVoxel(offset + Coords{ x[0] + q[0], x[1] + q[1], x[2] + q[2] });
						//b.type = VoxelType::AIR;
					}

						

					if (a.type == VoxelType::IGNORE_TYPE)
						a = emptyVoxel;
					if (b.type == VoxelType::IGNORE_TYPE)
						b = emptyVoxel;

                    if (!waterPass) {
                        if (a.type == VoxelType::WATER) {
                            setVoxel(m_waterPassGrid, x[0], x[1], x[2], a);
							a.type = VoxelType::AIR;
                            hasWater = true;
						} else {
							Voxel vox = a;
							vox.type = VoxelType::AIR;
							setVoxel(m_waterPassGrid, x[0], x[1], x[2], vox);
						}
                        if (b.type == VoxelType::WATER) {
                            setVoxel(m_waterPassGrid, x[0] + q[0], x[1] + q[1], x[2] + q[2], b);
							b.type = VoxelType::AIR;
                            hasWater = true;
						} else {
							Voxel vox = a;
							vox.type = VoxelType::AIR;
							setVoxel(m_waterPassGrid, x[0] + q[0], x[1] + q[1], x[2] + q[2], vox);
						}
                    }

                    const bool ba = a.type != VoxelType::AIR;
                    if (ba == (b.type != VoxelType::AIR)) {
						m_mask[counter] = emptyVoxel;
						m_light[counter] = a._light > b._light ? a._light : b._light;
                    } else if (ba) {
						m_mask[counter] = a;
						m_light[counter] = b._light;
                        m_offsetNormal[counter] = true;
                    } else{
						m_mask[counter] = b;
						m_light[counter] = a._light;
                        m_offsetNormal[counter] = false;
                    }
                }
            }
            ++x[axis];

            int width = 0, height = 0;
            counter = 0;
            for (int j = 0; j < CHUNK_SIZE; ++j) {
                for (int i = 0; i < CHUNK_SIZE;) {
                    Voxel c = m_mask[counter];
                    if (c.type != VoxelType::AIR) {
						/*
                        // Calcule de la largeur
                        for (width = 1; (c == m_mask[counter + width]) && (m_offsetNormal[counter] == m_offsetNormal[counter + width]) &&
                             i + width < CHUNK_SIZE; ++width) {
                        }

                        // Calcule de la hauteur
                        bool done = false;
                        for (height = 1; j + height < CHUNK_SIZE; ++height) {
                            for (int k = 0; k < width; ++k)
                                if (!(c == m_mask[counter + k + height * CHUNK_SIZE] && (m_offsetNormal[counter] == m_offsetNormal[counter + k + height * CHUNK_SIZE]))) {
                                    done = true;
                                    break;
                                }
                            if (done)
                                break;
                        }
						*/

                        // Ajout d'une face
                        x[u] = i;
                        x[v] = j;

                        int du[3] = {0}, dv[3] = {0};

                        int nIndex = axis * 2;
                        if (m_offsetNormal[counter]) {
							dv[v] = 1;// height;
							du[u] = 1;// width;
                            nIndex++;
                        } else {
							du[v] = 1;//height;
							dv[u] = 1;//width;
                        }
						

						VoxelTextureMap textureMap = VoxelTextures[(uint8)c.type];
						TextureID t = TextureID::ERROR_TEXTURE;

							 if (nIndex == 0) t = textureMap.right;
						else if (nIndex == 1) t = textureMap.left;
						else if (nIndex == 2) t = textureMap.bottom;
						else if (nIndex == 3) t = textureMap.top;
						else if (nIndex == 4) t = textureMap.front;
						else if (nIndex == 5) t = textureMap.back;						

						uint8 light = m_light[counter];

						vertices[vertexCount++] = getVertex(x[0], x[1], x[2], nIndex, t, light);
						vertices[vertexCount++] = getVertex(x[0] + dv[0], x[1] + dv[1], x[2] + dv[2], nIndex, t, light);
						vertices[vertexCount++] = getVertex(x[0] + du[0] + dv[0], x[1] + du[1] + dv[1], x[2] + du[2] + dv[2], nIndex, t, light);
						vertices[vertexCount++] = getVertex(x[0] + du[0] + dv[0], x[1] + du[1] + dv[1], x[2] + du[2] + dv[2], nIndex, t, light);
						vertices[vertexCount++] = getVertex(x[0] + du[0], x[1] + du[1], x[2] + du[2], nIndex, t, light);
						vertices[vertexCount++] = getVertex(x[0], x[1], x[2], nIndex, t, light);
						/*
                        for (int b = 0; b < width; ++b)
                            for (int a = 0; a < height; ++a)
                                m_mask[counter + b + a * CHUNK_SIZE] = emptyVoxel;
						
                        i += width; counter += width;
						*/
						m_mask[counter].type = VoxelType::AIR;
						i++;
						counter++;
                    } else {
                        ++i;
                        ++counter;
                    }
                }
            }
        }
    }
    if (!waterPass) {
		buffer->toUpOpaqueCount = vertexCount;
    } else {
		buffer->toUpWaterCount = vertexCount;
    }
    if ((!waterPass) && (hasWater)) {
		vertexCount += generate(m_waterPassGrid, chunkPos, buffer, vertices + vertexCount, true);
    } else if (!waterPass) {
		buffer->toUpWaterCount = 0;
    }
    return vertexCount;
}

Voxel MeshGenerator::getVoxel(Voxel* data, int i, int j, int k) {
    return data[i + CHUNK_SIZE * (j + CHUNK_SIZE * k)];
}

void MeshGenerator::setVoxel(Voxel* data, int i, int j, int k, Voxel voxel) {
    if ((i >= 0) && (j >= 0) && (k >= 0) &&
        (i < CHUNK_SIZE) && (j < CHUNK_SIZE) && (k < CHUNK_SIZE)) {
        data[i + CHUNK_SIZE * (j + CHUNK_SIZE * k)] = voxel;
    }
}

GLuint MeshGenerator::getVertex(int x, int y, int z, int normalIndex, TextureID tex, uint8 light) {
	/* Bit map 32bits
	0-15 Coords (3*5bits)
	16-23 Texture (8bits)
	24-26 Normal (3 bits)
	28-31 Light (5bits)
	*/
	return (light << 26) | (normalIndex << 23) | ((uint8)tex << 15) | (x << 10) | (y << 5) | (z);
}
