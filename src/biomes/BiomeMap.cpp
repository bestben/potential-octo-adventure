#include "BiomeMap.h"
#include "BiomeLayer.h"
#include <QtCore/QString>
#include "OpenSimplexNoise.hpp"



BiomeMap::BiomeMap() : mTunnelNoise(42)
{
	// Pluie
	mRainfall = new BiomeLayer(255.0, 0.0, 0.05, 0.0, 0.0, 0);

	// Temperatur entre -20°C et 30°C
	mTemperature = new BiomeLayer(60.0, -20.0, 0.005, 0.0, 0.0, 1);

	// Variation du sol
	mHeightmap = new BiomeLayer(40.0, 0.0, 0.025, 0.0, 0.0, 2);

	// Montagnes
	mMountains = new BiomeLayer(1.0, 0.0, 0.005, 0.0, 0.0, 3);

	mSharpHills = new BiomeLayer(5.0, 0.0, 0.005, 0.0, 0.0, 4);
}

double BiomeMap::getTunnelValue(const Coords& chunkId, int x, int y, int z) {

	int i = chunkId.i*CHUNK_SIZE + x;
	int j = chunkId.j*CHUNK_SIZE + y;
	int k = chunkId.k*CHUNK_SIZE + z;

	return (mTunnelNoise.value(i*0.05, j*0.05, k*0.05) + 1.0)*0.5; 
}


BiomeMap::~BiomeMap()
{
	delete mHeightmap;
	delete mMountains;
	delete mTemperature;
	delete mRainfall;
	delete mSharpHills;

}

int BiomeMap::getGroundLevel(const Coords& chunkId, int i, int k) {

	double value = (double)GROUND_LEVEL;
	value += mHeightmap->getValue(chunkId, i, k)*mMountains->getValue(chunkId, i, k);

	double sharpHill = mSharpHills->getValue(chunkId, i, k);
	if (sharpHill > 1.0) {
		value += sharpHill*sharpHill;
	}

	return (int)(value + 0.5); // Rounding
}

Voxel BiomeMap::getVoxelType(const Coords& chunkId, int i, int j, int k) {
	// TODO: Use other layers
	// TODO: Bias tunnels density near surface towards 100 to avoid holes in the ground

	int voxelHeight = chunkId.j*CHUNK_SIZE + j;

	if (voxelHeight == 0)
		return Voxel::ROCK;

	int terrainHeight = getGroundLevel(chunkId, i, k);


	double bias = clamp(range(voxelHeight-terrainHeight+5,0.0,20.0),0.0,1.0);


	if ((getTunnelValue(chunkId, i, j, k) + bias) > 0.3){
		if (voxelHeight < terrainHeight) {
			return Voxel::DIRT;
		}
		else if (voxelHeight == terrainHeight){
			return Voxel::GRASS;
		}
	}
	if (voxelHeight < 10)
		return Voxel::LAVA;
	return Voxel::AIR;
}
