#pragma once

#include <QtCore/qhash.h>
#include <QtCore/QString>

// TODO: Merge defines
#define CHUNK_SIZE 31
#define GROUND_LEVEL 128

typedef unsigned int uint;
typedef unsigned char Voxel;

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