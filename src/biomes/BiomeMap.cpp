#include "BiomeMap.h"
#include "BiomeLayer.h"
#include <QtCore/QString>
#include "OpenSimplexNoise.hpp"
#include <fstream>


BiomeMap::BiomeMap(int x, int y) : mTunnelNoise(42)
{
	mMapX = x;
	mMapY = y;

	double xOffset = mMapX*BIOMEMAP_SIZE;
	double yOffset = mMapY*BIOMEMAP_SIZE;


	// Pluie
	mRainfall = new BiomeLayer(1.0, 0.0, 0.05, xOffset, yOffset, 0);

	// Temperatur entre -20°C et 30°C
	mTemperature = new BiomeLayer(60.0, -20.0, 0.005, xOffset, yOffset, 1);

	// Variation du sol
	mHeightmap = new BiomeLayer(40.0, 0.0, 0.025, xOffset, yOffset, 2);

	// Montagnes
	mMountains = new BiomeLayer(1.0, 0.0, 0.005, xOffset, yOffset, 3);

	// Plateau, falaises
	mSharpHills = new BiomeLayer(5.0, 0.0, 0.05, xOffset, yOffset, 4);

	// Couche de terre avant la roche en sous-sol
	mDirtLayer = new BiomeLayer(20.0, 0.0, 0.05, xOffset, yOffset, 5);

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
	delete mDirtLayer;
}


Voxel BiomeMap::getVoxelType(const Coords& chunkId, int i, int j, int k) {

	// Coordonnées relative de ce chunks dans la map que cet objet représente
	Coords chunkIdInMap = chunkIdToChunkIdInMap(chunkId);

	Voxel result = Voxel::AIR;

	int voxelHeight = chunkId.j*CHUNK_SIZE + j;

	if (voxelHeight == 0){
		result = Voxel::ROCK;
		return result;
	}

	double temperature = mTemperature->getValue(chunkIdInMap, i, k);
	// On veut des montagnes pas hautes dans les deserts
	double biasDesert = 1.0 - clamp(range(temperature, 20.0, 30.0), 0.0, 1.0)*0.1;

	// Calcul de la hauteur de la surface
	double value = (double)GROUND_LEVEL;
	value += mHeightmap->getValue(chunkIdInMap, i, k)*mMountains->getValue(chunkIdInMap, i, k);

	double sharpHill = mSharpHills->getValue(chunkIdInMap, i, k);
	if (sharpHill > 1.0) {
		value += clamp(sharpHill*sharpHill, 0.0, 15.0);
	}

	value *= biasDesert;

	int terrainHeight = round(value);


	double bias = clamp(range(voxelHeight-terrainHeight+5,0.0,20.0),0.0,1.0);

	//double rain = mRainfall->getValue(chunkId, i, k);
	double dirt = mDirtLayer->getValue(chunkIdInMap, i, k);
	

	bool aboveGround = voxelHeight >= terrainHeight;
	bool atGround = voxelHeight == terrainHeight;
	bool inTunnel = (getTunnelValue(chunkId, i, j, k) + bias) > 0.3;


	int distanceFromSurface = voxelHeight - terrainHeight; // + : above, - : below
	int distanceFromSeaLevel = voxelHeight - GROUND_LEVEL;

	if (voxelHeight < 10  && !inTunnel)
		return Voxel::LAVA;

	// Tunnels
	if (!aboveGround && inTunnel) {
		if (distanceFromSurface < round(-dirt)) {
			return Voxel::ROCK;
		}
		result = Voxel::DIRT;
	}

	//Surface
	if (atGround && inTunnel)
		result = Voxel::GRASS;

	
		

	if (!atGround && aboveGround && distanceFromSeaLevel<SEA_HEIGHT)
		result = Voxel::WATER;

/*	if ((distanceFromSeaLevel == SEA_HEIGHT || distanceFromSeaLevel == SEA_HEIGHT-1) && result == Voxel::GRASS) {
		result = Voxel::SAND;
	}*/
	if (temperature > 20){
		if (result == Voxel::GRASS || result == Voxel::DIRT) {
			result = Voxel::SAND;
		}
		if (result == Voxel::WATER)
			result = Voxel::SAND;

		/*if (!atGround && aboveGround && distanceFromSeaLevel<SEA_HEIGHT)/2
			result = Voxel::SAND;*/
	}


	return result;
}
