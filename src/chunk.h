#pragma once

#include <QtCore/qhash.h>
#include <QtCore/QString>

#define CHUNK_NUMBER 700
#define VBO_NUMBER 700

#define CHUNK_SIZE 31
#define CHUNK_SCALE 5

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
};

#define GROUND_LEVEL 128

typedef unsigned int uint;
typedef unsigned char Voxel;

// TODO: Enum voxel types

struct MapIndex
{
	int a;
	int b;
};

inline uint qHash(MapIndex key) {
	QString str = "";
	str += key.a;
	str += "-";
	str += key.b;
	return qHash(str);
}

inline bool operator==(MapIndex lhs, MapIndex rhs){
	return (lhs.a == rhs.a) && (lhs.b == rhs.b);
}


