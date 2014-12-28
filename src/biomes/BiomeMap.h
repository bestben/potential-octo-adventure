#pragma once

#include <QtCore/QHash>
#include <memory>
#include <fstream>
#include "../chunk.h"
#include "../libnoise/noise.h"

class BiomeLayer;

class BiomeMap
{
public:
	BiomeMap(int mapX, int mapY);
	VoxelType getVoxelType(const Coords& chunkId, int i, int j, int k);
	~BiomeMap();

private:

	int mMapX;
	int mMapY;

	noise::module::Perlin mTunnelNoise;

	inline double getTunnelValue(const Coords& chunkId, int x, int y, int z);

	inline double getValue(double* data, Coords chunkIdInMap, int i, int k);

	double *mBiomeSelector;
	double *mFlatlands;
	double *mMountains;
	double *mTemperature;
	double *mDirtLayer;
	double *mHeightmap;

	std::ofstream mLog;
};

