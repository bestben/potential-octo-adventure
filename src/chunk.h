#pragma once

#include <QtCore/qhash.h>
#include <QtCore/QString>

#define CHUNK_NUMBER 700
#define VBO_NUMBER 700

#define CHUNK_SIZE 31
#define CHUNK_SCALE 6

#define VIEW_SIZE 4


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


enum class Voxel : unsigned char
{
	AIR = 0,
	GRASS,
	DIRT,
	ROCK,
	STONE,
	SAND,
	GRAVEL,
	TRUNK,
	LAVA,
	COUNT // On ajoute un elmeent pour avoir la taille de l'enum
};

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


inline double lerp(double v0, double v1, double t) {
	return (1 - t)*v0 + t*v1;
}

inline double clamp(double value, double min, double max){
	return value > max ? max : (value < min ? min : value);
}

inline double range(double value, double min, double max) {
	return (value - min) / (max - min);
}