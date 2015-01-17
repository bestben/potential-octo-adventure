#pragma once
#include <QtCore/QString>
#include <QtCore/QHash>
#include <QtGui/QVector3D>
#include <cstdlib>

// Operations de division avec correction de la troncature dans les nombres négatifs
inline int div_floor(int x, int y) {

    div_t res = div(x, y);
    if ((res.rem != 0) && ((res.rem<0) != (y<0))) --res.quot;
    return res.quot;
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

namespace std {
    template <>
    struct hash<Coords>
    {
        std::size_t operator()(const Coords& c) const
        {
            int hash = 23;
            hash = hash * 31 + c.i;
            hash = hash * 31 + c.j;
            hash = hash * 31 + c.k;
            return hash;
        }
    };
}

// Permet d'utiliser Coords dans un container qt
inline uint qHash(Coords c) {
	
	int hash = 23;
	hash = hash * 31*31*31 + c.i;
	hash = hash * 31*31 + c.j;
	hash = hash * 31 + c.k;
	return hash;

	/*QString str = "";
	str += c.i;
	str += "-";
	str += c.j;
	str += "-";
	str += c.k;
	return qHash(str);*/
}

inline bool operator< (const Coords& lhs, const Coords& rhs) {
    if (lhs.i != rhs.i) {
        return lhs.i < rhs.i;
    }
    if (lhs.j != rhs.j) {
        return lhs.j < rhs.j;
    }
    if (lhs.k != rhs.k) {
        return lhs.k < rhs.k;
    }
    return false;
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


inline Coords GetChunkRelPosInBiomeMap(Coords const& chunkId) {
	return{ mod_floor(chunkId.i, BIOMEMAP_CHUNKS), mod_floor(chunkId.j, BIOMEMAP_CHUNKS), mod_floor(chunkId.k, BIOMEMAP_CHUNKS) };
}

inline Coords GetVoxelRelPos(Coords const& c) {
	return{ mod_floor(c.i, CHUNK_SIZE), mod_floor(c.j, CHUNK_SIZE), mod_floor(c.k, CHUNK_SIZE) };
}

inline Coords GetChunkPosFromVoxelPos(Coords const& c) {
	return{ div_floor(c.i, CHUNK_SIZE), div_floor(c.j, CHUNK_SIZE), div_floor(c.k, CHUNK_SIZE) };
}

inline Coords GetChunkBiomeMap(Coords const& chunkId) {
	return{ div_floor(chunkId.i, BIOMEMAP_CHUNKS), 0, div_floor(chunkId.k, BIOMEMAP_CHUNKS) };
}

inline Coords GetVoxelPosFromWorldPos(const QVector3D &pos) {
	return{ div_floor(pos.x(), (CHUNK_SCALE)), div_floor(pos.y(), (CHUNK_SCALE)), div_floor(pos.z(), (CHUNK_SCALE)) };
}

inline QVector3D voxelToWorld(const Coords& pos) {
        return QVector3D((pos.i + 0.5) * CHUNK_SCALE, (pos.j + 0.5) * CHUNK_SCALE, (pos.k + 0.5) * CHUNK_SCALE);
}

inline int IndexVoxelRelPos(Coords const& c) {
	return ((c.k * CHUNK_SIZE) + c.j)*CHUNK_SIZE + c.i;
}

inline bool atChunkBounds(Coords const& c) {
	return c.i == 0 || c.i == CHUNK_SIZE - 1 ||
		c.j == 0 || c.j == CHUNK_SIZE - 1 ||
		c.k == 0 || c.k == CHUNK_SIZE - 1;
}
