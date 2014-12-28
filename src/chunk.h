#pragma once

#include <QtCore/qhash.h>
#include <QtCore/QString>

#define CHUNK_NUMBER 700
#define VBO_NUMBER 700

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

typedef unsigned int uint;
//typedef unsigned char Voxel;
//typedef std::tuple<int, int, int> Coords;

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

enum class VoxelType : unsigned char
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

typedef unsigned int Voxel;

enum class TextureID : uint
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

inline Coords worldCoordsToChunkCoords(Coords c) {
	int i = c.i % CHUNK_SIZE;
	int j = c.j % CHUNK_SIZE;
	int k = c.k % CHUNK_SIZE;
	return{ i<0 ? i + CHUNK_SIZE : i, j<0 ? j + CHUNK_SIZE : j, k<0 ? k + CHUNK_SIZE : k };
}

inline Coords chunkIdToMapId(Coords chunkId) {
	return{ div_floor(chunkId.i, BIOMEMAP_CHUNKS), 0, div_floor(chunkId.k, BIOMEMAP_CHUNKS) };
}

// Get the bits XXXX XXXX 0000 0000
inline VoxelType getVoxelType(Voxel v) {
	return (VoxelType)((v >> 8) & 0xF);
}

// Get the bits XXXX XXXX 0000 0000
inline void setVoxelType(Voxel *v, VoxelType type) {
	*v = (*v & 0xF) | ((unsigned char)type << 8);
}

// Get the bits 0000 0000 XXXX XXXX 
inline unsigned char getVoxelLight(Voxel v) {
	return (v & 0xF);
}

// Get the bits 0000 0000 XXXX XXXX
inline void setVoxelLight(Voxel *v, unsigned char val) {
	*v = (*v & 0xF0) | val;
}