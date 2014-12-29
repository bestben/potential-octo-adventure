#pragma once

#include <QtCore/qhash.h>
#include <QtCore/QString>
#include <QtGui/QVector3D>

#include <cstdint>

#define CHUNK_NUMBER 1024
#define VBO_NUMBER 1024

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
};

#define GROUND_LEVEL 128
#define SEA_HEIGHT 10

typedef int8_t int8;
typedef uint8_t uint8;
typedef int16_t int16;
typedef uint16_t uint16;
typedef int32_t int32;
typedef uint32_t uint32;
typedef int64_t int64;
typedef uint64_t uint64;

struct Coords
{
	int i;
	int j;
	int k;
};


// TODO: Enum voxel types

inline uint qHash(Coords c) {
	QString str = "";
	str += c.i;
	str += "-";
	str += c.j;
	str += "-";
	str += c.k;
	return qHash(str);
}

inline bool operator==(Coords lhs, Coords rhs){
	return (lhs.i == rhs.i) && (lhs.j == rhs.j) && (lhs.k == rhs.k);
}

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


inline int div_floor(int x, int y) {
	int q = x / y;
	int r = x%y;
	if ((r != 0) && ((r<0) != (y<0))) --q;
	return q;
}

inline double lerp(double v0, double v1, double t) {
	return (1 - t)*v0 + t*v1;
}

inline double clamp(double value, double min, double max){
	return value > max ? max : (value < min ? min : value);
}

inline double range(double value, double min, double max) {
	return (value - min) / (max - min);
}

inline Coords chunkIdToChunkIdInMap(Coords chunkId) {
	int i = chunkId.i % BIOMEMAP_CHUNKS;
	int k = chunkId.k % BIOMEMAP_CHUNKS;
	return{ i<0 ? i + BIOMEMAP_CHUNKS : i, chunkId.j, k<0 ? k + BIOMEMAP_CHUNKS  : k};
}

inline Coords voxelCoordsToChunkCoords(Coords c) {
	int i = c.i % CHUNK_SIZE;
	int j = c.j % CHUNK_SIZE;
	int k = c.k % CHUNK_SIZE;
	return{ i<0 ? i + CHUNK_SIZE : i, j<0 ? j + CHUNK_SIZE : j, k<0 ? k + CHUNK_SIZE : k };
}

inline Coords voxelGetChunk(Coords c) {
	return{ div_floor(c.i, CHUNK_SIZE), div_floor(c.j, CHUNK_SIZE), div_floor(c.k, CHUNK_SIZE) };
}

inline Coords chunkIdToMapId(Coords chunkId) {
	return{ div_floor(chunkId.i, BIOMEMAP_CHUNKS), 0, div_floor(chunkId.k, BIOMEMAP_CHUNKS) };
}

inline Coords worldToVoxel(const QVector3D &pos) {
	return{ div_floor(pos.x(), (CHUNK_SCALE)), div_floor(pos.y(), (CHUNK_SCALE)), div_floor(pos.z(), (CHUNK_SCALE)) };
}
