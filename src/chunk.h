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

#define FREE_BUFFERS_THRESHOLD 32

#define CHUNK_SIZE 16
#define CHUNK_SCALE 5

#define VIEW_SIZE 4

#define BIOMEMAP_CHUNKS 16
#define BIOMEMAP_SIZE CHUNK_SIZE*BIOMEMAP_CHUNKS

#define BIOMES_COUNT 4

#define WORLD_HEIGHT 4

#define SUN_LIGHT 31
#define MAX_LIGHT 30

#define GROUND_LEVEL (WORLD_HEIGHT*CHUNK_SIZE)/3
#define SEA_HEIGHT 10

#ifdef QT_DEBUG
#define Assert(Expression) if(!(Expression)) {*(int *)0 = 0;}
#else
#define Assert(Expression)
#endif

#include "coords.h"

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
	IGNORE_TYPE,
	COUNT 
};

inline uint8 lightSource(VoxelType type) {
	if (type == VoxelType::LAVA)
		return MAX_LIGHT;

	return 0;
}

struct Voxel
{
	Voxel(){ type = VoxelType::AIR; _light = 0; }
	Voxel(VoxelType t){ type = t; _light = 0; }
	VoxelType type : 8;
	uint8 _light : 8;

	uint8 getLight() {
		uint8 source = lightSource(type);
		return source > _light ? source : _light;
	}
};






#define NO_CHANGE -1

#define IGNORE_VOXEL Voxel(VoxelType::IGNORE_TYPE)

struct Chunk {

	Chunk() {
		inQueue = false;
		onlyAir = false;

		isDirty = false;
		isLightDirty = false;

		ready = false;

		chunkBufferIndex = -1;
		vboIndex = -1;
		visible = false;

		generated = false;

	}

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
	bool isLightDirty;

	bool generated;

	bool inQueue;

	bool onlyAir;


};


struct ChunkPile{
	Chunk* pile[WORLD_HEIGHT];

	// Cache access
	Chunk* _lastChunk;
	uint16 _lastChunkId;

	uint16 pileI;
	uint16 pileJ;

	bool modified = false;
};


inline bool operator==(const Voxel &lhs, const Voxel &rhs){
	return lhs.type == rhs.type && lhs._light == rhs._light;
}

inline bool isOpaque(const Voxel& v) {
	return v.type != VoxelType::AIR && v.type != VoxelType::WATER && v.type != VoxelType::LAVA;
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



inline uint8 reduce_light(uint8 light)
{
	if (light == 0)
		return 0;
	if (light >= MAX_LIGHT)
		return MAX_LIGHT - 1;
	return light - 1;
}

inline uint8 unreduce_light(uint8 light)
{
	if (light == 0)
		return 0;
	if (light == MAX_LIGHT)
		return light;
	return light + 1;
}



void initializeTextureMaps();

TextureID getTexture(VoxelType type, int side);
VoxelTextureMap getTextureMap(VoxelType type);

