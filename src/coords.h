#pragma once
#include <QtCore/QString>
#include <QtCore/QHash>

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

inline bool operator==(const Coords &l, const Coords &r){
	return (l.i == r.i) && (l.j == r.j) && (l.k == r.k);
}
inline bool operator!=(const Coords &l, const Coords &r){
	return !(l == r);
}
inline Coords operator+(Coords &l, Coords &r){
	return{ l.i + r.i, l.j + r.j, l.k + r.k };
}
inline Coords operator-(Coords &l, Coords &r){
	return{ l.i - r.i, l.j - r.j, l.k - r.k };
}
inline Coords operator+=(Coords &l, Coords &r){
	return l + r;
}
inline Coords operator-=(Coords &l, Coords &r){
	return l - r;
}
inline Coords operator*(Coords &l, int s){
	return{ l.i * s, l.j * s, l.k * s };
}
inline Coords operator*(int s, Coords &r){
	return r*s;
}
inline Coords operator*=(Coords &l, int s){
	return l*s;
}
inline Coords operator/(Coords &l, int d){
	return{ div_floor(l.i, d), div_floor(l.j, d), div_floor(l.k, d) };
}
inline Coords operator/=(Coords &l, int d){
	return l / d;
}
inline Coords operator%(Coords &l, int d){
	return{ mod_floor(l.i, d), mod_floor(l.j, d), mod_floor(l.k, d) };
}
inline Coords operator%=(Coords &l, int d){
	return l%d;
}


inline Coords chunkIdToChunkIdInMap(Coords chunkId) {
	int i = chunkId.i % BIOMEMAP_CHUNKS;
	int k = chunkId.k % BIOMEMAP_CHUNKS;
	return{ i<0 ? i + BIOMEMAP_CHUNKS : i, chunkId.j, k<0 ? k + BIOMEMAP_CHUNKS : k };
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