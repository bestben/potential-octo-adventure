#pragma once

#include <QtCore/QHash>
#include "BiomeLayer.h"
#include <memory>
#include <fstream>

class BiomeLayer;

class BiomeMap
{
public:
	BiomeMap(int x, int y);
	Voxel getVoxelType(const Coords& chunkId, int i, int j, int k);
	~BiomeMap();

private:

	int mMapX;
	int mMapY;

	double getTunnelValue(const Coords& chunkId, int x, int y, int z);

	OpenSimplexNoise mTunnelNoise;

	BiomeLayer *mHeightmap;
	BiomeLayer *mMountains;
	BiomeLayer *mTemperature;
	BiomeLayer *mRainfall;
	BiomeLayer *mSharpHills;
	BiomeLayer *mDirtLayer;	

	std::ofstream mLog;
};

