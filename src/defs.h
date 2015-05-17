#pragma once

// TODO: Merge defines
#define CHUNK_SIZE 31
#define GROUND_LEVEL 128

typedef unsigned int uint;

struct MapIndex
{
	int a;
	int b;
};

inline bool operator==(MapIndex lhs, MapIndex rhs){
	return (lhs.a == rhs.a) && (lhs.b == rhs.b);
}
