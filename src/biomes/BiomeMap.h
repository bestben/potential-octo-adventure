#pragma once

#include <QtCore/QHash>
#include "BiomeLayer.h"
#include <memory>

class BiomeLayer;

class BiomeMap
{
public:
	BiomeMap();
	int getGroundLevel(const Coords& chunkId, int i, int k);
	Voxel getVoxelType(const Coords& chunkId, int i, int j, int k);
	~BiomeMap();

private:

	double getTunnelValue(const Coords& chunkId, int x, int y, int z);

	OpenSimplexNoise mTunnelNoise;

	BiomeLayer *mHeightmap;
	BiomeLayer *mMountains;
	BiomeLayer *mTemperature;
	BiomeLayer *mRainfall;
	BiomeLayer *mSharpHills;
};

