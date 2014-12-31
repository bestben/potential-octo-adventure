#pragma once

#include <QtGui/QVector3D>
#include <cstdint>

typedef int8_t int8;
typedef uint8_t uint8;
typedef int16_t int16;
typedef uint16_t uint16;
typedef int32_t int32;
typedef uint32_t uint32;
typedef int64_t int64;
typedef uint64_t uint64;

#define CHUNK_NUMBER 2048
#define VBO_NUMBER 2048

#define CHUNK_SIZE 31
#define CHUNK_SCALE 5

#define VIEW_SIZE 4

#define BIOMEMAP_CHUNKS 8
#define BIOMEMAP_SIZE CHUNK_SIZE*BIOMEMAP_CHUNKS

#define BIOMES_COUNT 4

#ifdef QT_DEBUG
#define Assert(Expression) if(!(Expression)) {*(int *)0 = 0;}
#else
#define Assert(Expression)
#endif

#include "coords.h"

struct Chunk {
    // Les coordonnées du chunk
    int i;
    int j;
    int k;

    bool visible;
    float distanceFromCamera;

    int chunkBufferIndex;
    int vboIndex;
	bool ready;
	bool isDirty;

	Chunk* chunkXP;
	Chunk* chunkXM;
	Chunk* chunkYP;
	Chunk* chunkYM;
	Chunk* chunkZP;
	Chunk* chunkZM;

	bool inQueue;
};

#define GROUND_LEVEL 128
#define SEA_HEIGHT 10




enum class VoxelType : uint8
{
	AIR = 0,
	GRASS,
	DIRT,
	ROCK,
	STONE,
	SAND,
	GRAVEL,
	TRUNK,
	WATER,
	LAVA,
	LEAVES,
	COUNT // On ajoute un elmeent pour avoir la taille de l'enum
};

//typedef unsigned int Voxel;

struct Voxel
{
	VoxelType type : 8;
	uint8 sunLight : 4;
	uint8 torchLight : 4;
};

inline bool operator==(const Voxel &lhs, const Voxel &rhs){
	return lhs.type == rhs.type && lhs.sunLight == rhs.sunLight && lhs.torchLight == rhs.torchLight;
}

inline bool isOpaque(const Voxel& v) {
	return v.type != VoxelType::AIR && v.type != VoxelType::WATER;
}

enum class TextureID : uint8
{
	GRASS = 0,
	ROCK = 1,
	DIRT = 2,
	GRASS_SIDE = 3,
	WOOD_PLANK = 4,
	STONE_SIDE = 5,
	STONE_TOP = 6,
	SAND = 18,
	GRAVEL = 19,
	TRUNK_SIDE = 20,
	TRUNK_TOP = 21,
	LEAVES = 52,
	WATER = 223,
	LAVA = 255,
	ERROR_TEXTURE = 22
};

struct VoxelTextureMap
{
	TextureID top;
	TextureID bottom;
	TextureID left;
	TextureID right;
	TextureID front;
	TextureID back;
};

#define FULL_BLOCK(name) VoxelTextures[(uint)VoxelType::name] = { TextureID::name, TextureID::name, TextureID::name, TextureID::name, TextureID::name, TextureID::name };


inline double lerp(double v0, double v1, double t) {
	return (1 - t)*v0 + t*v1;
}

inline double clamp(double value, double min, double max){
	return value > max ? max : (value < min ? min : value);
}

inline double range(double value, double min, double max) {
	return (value - min) / (max - min);
}


#define BIND_LIGHT_MAP_SIDE(name, nb) if (chunk->name) { if(chunk->name->vboIndex != -1) {sideBuffer = m_oglBuffers + chunk->name->vboIndex; sideBuffer->texture_light->bind(nb); gl->glTexBuffer(GL_TEXTURE_BUFFER, GL_R16UI, sideBuffer->vbo_light); } }

