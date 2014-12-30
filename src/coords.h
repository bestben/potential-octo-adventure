#pragma once
#include <QtCore/QString>
#include <QtCore/QHash>

// Operations de division avec correction de la troncature dans les nombres négatifs
inline int div_floor(int x, int y) {
	int q = x / y;
	int r = x%y;
	if ((r != 0) && ((r<0) != (y<0))) --q;
	return q;
}

inline int mod_floor(int x, int y) {
	int r = x%y;
	return (r < 0) ? r + y : r;
}

// Coordonnées entières en 3D (voxel, chunk, ...)
struct Coords
{
	int i;
	int j;
	int k;

	inline int operator[](int axis)
	{
		axis %= 3;
		if (axis == 0) return i;
		if (axis == 1) return j;
		if (axis == 2) return k;
	}

	Coords& operator+=(const Coords& rhs) { i += rhs.i; j += rhs.j; k += rhs.k; return *this; }
	Coords& operator+=(const int& v) { i += v; j += v; k += v; return *this; }

	Coords& operator-=(const Coords& rhs) { i -= rhs.i; j -= rhs.j; k -= rhs.k; return *this; }
	Coords& operator-=(const int& v) { i -= v; j -= v; k -= v; return *this; }

	Coords& operator*=(const int& v) { i *= v; j *= v; k *= v; return *this; }
	Coords& operator/=(const int& v) { i = div_floor(i, v); j = div_floor(j, v); k = div_floor(k, v); return *this; }
	Coords& operator%=(const int& v) { i = mod_floor(i, v); j = mod_floor(j, v); k = mod_floor(k, v); return *this; }

};

// Permet d'utiliser Coords dans un container qt
inline uint qHash(Coords c) {
	QString str = "";
	str += c.i;
	str += "-";
	str += c.j;
	str += "-";
	str += c.k;
	return qHash(str);
}

inline bool operator==(Coords const& l, Coords const& r){
	return (l.i == r.i) && (l.j == r.j) && (l.k == r.k);
}
inline bool operator!=(Coords const& l, Coords const& r){
	return !(l == r);
}
inline Coords operator+(Coords l, Coords const& r){
	l += r;
	return l;
}
inline Coords operator+(Coords l, int const& r){
	l += r;
	return l;
}
inline Coords operator-(Coords l, Coords const& r){
	l -= r;
	return l;
}
inline Coords operator-(Coords l, int const& r){
	l -= r;
	return l;
}
inline Coords operator*(Coords l, int s){
	l *= s;
	return l;
}
inline Coords operator*(int s, Coords r){
	return r*s;
}
inline Coords operator/(Coords l, int d){
	l /= d;
	return l;
}
inline Coords operator%(Coords l, int d){
	l %= d;
	return l;
}


inline Coords chunkIdToChunkIdInMap(Coords const& chunkId) {
	int i = chunkId.i % BIOMEMAP_CHUNKS;
	int k = chunkId.k % BIOMEMAP_CHUNKS;
	return{ i<0 ? i + BIOMEMAP_CHUNKS : i, chunkId.j, k<0 ? k + BIOMEMAP_CHUNKS : k };
}

inline Coords voxelCoordsToChunkCoords(Coords const& c) {
	int i = c.i % CHUNK_SIZE;
	int j = c.j % CHUNK_SIZE;
	int k = c.k % CHUNK_SIZE;
	return{ i<0 ? i + CHUNK_SIZE : i, j<0 ? j + CHUNK_SIZE : j, k<0 ? k + CHUNK_SIZE : k };
}

inline Coords voxelGetChunk(Coords const& c) {
	return{ div_floor(c.i, CHUNK_SIZE), div_floor(c.j, CHUNK_SIZE), div_floor(c.k, CHUNK_SIZE) };
}

inline Coords chunkIdToMapId(Coords const& chunkId) {
	return{ div_floor(chunkId.i, BIOMEMAP_CHUNKS), 0, div_floor(chunkId.k, BIOMEMAP_CHUNKS) };
}

inline Coords worldToVoxel(const QVector3D &pos) {
	return{ div_floor(pos.x(), (CHUNK_SCALE)), div_floor(pos.y(), (CHUNK_SCALE)), div_floor(pos.z(), (CHUNK_SCALE)) };
}

inline int getIndexInChunkData(Coords const& c) {
	return ((c.k * CHUNK_SIZE) + c.j)*CHUNK_SIZE + c.i;
}