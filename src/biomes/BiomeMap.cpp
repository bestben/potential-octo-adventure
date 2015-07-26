#include "BiomeMap.h"
#include <fstream>
#include "../libnoise/noise.h"

using namespace noise;

BiomeMap::BiomeMap(int mapX, int mapY, int worldSeed)
{
	mWorldSeed = worldSeed;
	mMapX = mapX;
	mMapY = mapY;
	

	double xOffset = mMapX*BIOMEMAP_SIZE;
	double zOffset = mMapY*BIOMEMAP_SIZE;

	mTunnelNoise.SetSeed(worldSeed+42);
	mTunnelNoise.SetFrequency(0.05);
	mTunnelNoise.SetOctaveCount(1);
	mTunnelNoise.SetNoiseQuality(NoiseQuality::QUALITY_FAST);

	// Variation des biomes
	module::Voronoi biomeSelectorNoise;
	biomeSelectorNoise.SetSeed(worldSeed + 1337);
	biomeSelectorNoise.SetDisplacement(BIOMES_COUNT);
	biomeSelectorNoise.SetFrequency(0.01);
	module::ScaleBias biasBiomeSelector;
	biasBiomeSelector.SetSourceModule(0, biomeSelectorNoise);
	biasBiomeSelector.SetScale(0.5);
	biasBiomeSelector.SetBias(BIOMES_COUNT*0.5);

	// Montagnes
	mMountains = new double[BIOMEMAP_SIZE*BIOMEMAP_SIZE];
	module::Perlin moutainsNoise;
	moutainsNoise.SetNoiseQuality(noise::NoiseQuality::QUALITY_FAST);
	moutainsNoise.SetSeed(worldSeed + 0);
	moutainsNoise.SetOctaveCount(4);
	moutainsNoise.SetFrequency(0.025);
	module::ScaleBias biasMoutains;
	biasMoutains.SetSourceModule(0,moutainsNoise);
	biasMoutains.SetScale(30);
	biasMoutains.SetBias(30);

	for (int z = 0; z < BIOMEMAP_SIZE; ++z) {
		for (int x = 0; x < BIOMEMAP_SIZE; ++x) {
			mMountains[z*BIOMEMAP_SIZE + x] = biasMoutains.GetValue((x + xOffset), 0.0, (z + zOffset));
		}
	}

	//Flatlands
	mFlatlands = new double[BIOMEMAP_SIZE*BIOMEMAP_SIZE];
	module::Perlin flatNoise;
	flatNoise.SetNoiseQuality(noise::NoiseQuality::QUALITY_FAST);
	flatNoise.SetSeed(worldSeed + 1);
	flatNoise.SetOctaveCount(3);
	flatNoise.SetFrequency(0.01);
	module::ScaleBias biasFlatlands;
	biasFlatlands.SetSourceModule(0, flatNoise);
	biasFlatlands.SetScale(5);
	biasFlatlands.SetBias(5);
	for (int z = 0; z < BIOMEMAP_SIZE; ++z) {
		for (int x = 0; x < BIOMEMAP_SIZE; ++x) {
			mFlatlands[z*BIOMEMAP_SIZE + x] = biasFlatlands.GetValue((x + xOffset), 0.0, (z + zOffset));
		}
	}

	// Biome selector
	module::Select biomeSelector;
	biomeSelector.SetSourceModule(0, biasFlatlands);
	biomeSelector.SetSourceModule(1, biasMoutains);
	biomeSelector.SetControlModule(biasBiomeSelector);
	biomeSelector.SetBounds(3.0, 1000.0); // In bounds => 1 
	biomeSelector.SetEdgeFalloff(20.0);

	// Heightmap
	mHeightmap = new double[BIOMEMAP_SIZE*BIOMEMAP_SIZE];
	for (int z = 0; z < BIOMEMAP_SIZE; ++z) {
		for (int x = 0; x < BIOMEMAP_SIZE; ++x) {
			mHeightmap[z*BIOMEMAP_SIZE + x] = biomeSelector.GetValue((x + xOffset), 0.0, (z + zOffset));
		}
	}
	
	// Dirtlayer
	mDirtLayer = new double[BIOMEMAP_SIZE*BIOMEMAP_SIZE];
	module::Perlin dirtNoise;
	dirtNoise.SetNoiseQuality(noise::NoiseQuality::QUALITY_FAST);
	dirtNoise.SetSeed(worldSeed + 5);
	dirtNoise.SetOctaveCount(2);
	dirtNoise.SetFrequency(0.05);
	module::ScaleBias dirtFinalNoise;
	dirtFinalNoise.SetSourceModule(0, dirtNoise);
	dirtFinalNoise.SetScale(10);
	dirtFinalNoise.SetBias(10);
	for (int z = 0; z < BIOMEMAP_SIZE; ++z) {
		for (int x = 0; x < BIOMEMAP_SIZE; ++x) {
			mDirtLayer[z*BIOMEMAP_SIZE + x] = dirtFinalNoise.GetValue((x + xOffset), 0.0, (z + zOffset));
		}
	}

	// Temperature
	mTemperature = new double[BIOMEMAP_SIZE*BIOMEMAP_SIZE];
	module::Perlin tempNoise;
	tempNoise.SetNoiseQuality(noise::NoiseQuality::QUALITY_FAST);
	tempNoise.SetSeed(worldSeed + 6);
	tempNoise.SetOctaveCount(2);
	tempNoise.SetFrequency(0.005);
	module::ScaleBias tempFinalNoise;
	tempFinalNoise.SetSourceModule(0, tempNoise);
	tempFinalNoise.SetScale(25);
	tempFinalNoise.SetBias(5);
	for (int z = 0; z < BIOMEMAP_SIZE; ++z) {
		for (int x = 0; x < BIOMEMAP_SIZE; ++x) {
			mTemperature[z*BIOMEMAP_SIZE + x] = tempFinalNoise.GetValue((x + xOffset), 0.0, (z + zOffset));
		}
	}

}


inline double BiomeMap::getValue(double* data, Coords chunkIdInMap, int i, int k) {

	int x = i + chunkIdInMap.i*CHUNK_SIZE;
	int z = k + chunkIdInMap.k*CHUNK_SIZE;

	return data[z*BIOMEMAP_SIZE + x];
}

inline double BiomeMap::getTunnelValue(const Coords& chunkId, int x, int y, int z) {

	
	int i = chunkId.i*CHUNK_SIZE + x;
	int j = chunkId.j*CHUNK_SIZE + y;
	int k = chunkId.k*CHUNK_SIZE + z;

	return (mTunnelNoise.GetValue(i, j, k)+1.0)*0.5;
}


BiomeMap::~BiomeMap()
{
	delete mFlatlands;
	delete mMountains;
	delete mTemperature;
	delete mDirtLayer;
	delete mHeightmap;
}

int BiomeMap::getGroundLevel(const Coords& chunkId, int i, int k){
	Coords chunkIdInMap = GetChunkRelPosInBiomeMap(chunkId);
	double temperature = getValue(mTemperature, chunkIdInMap, i, k);
	double biasDesert = 1.0 - clamp(range(temperature, 20.0, 35.0), 0.0, 1.0);
	double value = (double)GROUND_LEVEL;
	value += getValue(mHeightmap, chunkIdInMap, i, k)*biasDesert;

    int terrainHeight = (int)round(value);
	int j = terrainHeight;
	for (;; --j) {
		double bias = clamp(range(j - GROUND_LEVEL - 12, 0.0, 20.0), -0.05, 1.0);
		double tunnel = getTunnelValue(chunkId, i, j - chunkId.j*CHUNK_SIZE, k);
		bool inTunnel = (tunnel + bias) > 0.2;

        if (inTunnel || (j == 1));
			break;
	}
	

	return j-1;
}


VoxelType BiomeMap::getVoxelType(const Coords& chunkId, int i, int j, int k) {

	// Coordonnées relative de ce chunks dans la map que cet objet représente
	Coords chunkIdInMap = GetChunkRelPosInBiomeMap(chunkId);
	int voxelHeight = chunkId.j*CHUNK_SIZE + j;

	VoxelType result = VoxelType::AIR;

	if (voxelHeight == 0) {
		result = VoxelType::ROCK;
		return result;
	}

	double temperature = getValue(mTemperature, chunkIdInMap, i, k);
	VoxelType replace = temperature > 20 ? VoxelType::SAND : VoxelType::GRAVEL;

	double biasDesert = 1.0 - clamp(range(temperature, 20.0, 35.0), 0.0, 1.0);
	double value = (double)GROUND_LEVEL;
	value += getValue(mHeightmap, chunkIdInMap, i, k)*biasDesert;

	
	// Calcul de la hauteur de la surface
	int terrainHeight = round(value);


	double bias = clamp(range(voxelHeight - GROUND_LEVEL - 12, 0.0, 20.0), -0.05, 1.0);

	//double rain = mRainfall->getValue(chunkId, i, k);
	double dirt = getValue(mDirtLayer, chunkIdInMap, i, k);
	

	bool aboveGround = voxelHeight >= terrainHeight;
	bool atGround = voxelHeight == terrainHeight;

	int distanceFromSurface = voxelHeight - terrainHeight; // + : above, - : below
	int distanceFromSeaLevel = voxelHeight - GROUND_LEVEL;

	if (!atGround && aboveGround && distanceFromSeaLevel >= SEA_HEIGHT)
		return result;

	double tunnel = getTunnelValue(chunkId, i, j, k);
	bool inTunnel = (tunnel + bias) > 0.2;
	bool inTunnelBorder = (tunnel + bias) > 0.2 && (tunnel + bias) < 0.25;
	
	if (!atGround && aboveGround && distanceFromSeaLevel<SEA_HEIGHT && inTunnel)
		result = VoxelType::WATER;

	if (voxelHeight < 10  && !inTunnel)
		return VoxelType::LAVA;

	// Tunnels
	if (!aboveGround && inTunnel) {
		if (distanceFromSurface < round(-dirt)) {
			return VoxelType::ROCK;
		}
		result = VoxelType::DIRT;
	}

	//Surface
	if (atGround && inTunnel)
		result = VoxelType::GRASS;
	if (result == VoxelType::GRASS && distanceFromSeaLevel < SEA_HEIGHT)
		result = replace;
	if ((atGround||result==VoxelType::WATER)&&inTunnelBorder)
		result = replace;

/*	if ((distanceFromSeaLevel == SEA_HEIGHT || distanceFromSeaLevel == SEA_HEIGHT-1) && result == Voxel::GRASS) {
		result = Voxel::SAND;
	}*/
	
	if (temperature > 18) {
		if (result == VoxelType::GRASS || result == VoxelType::DIRT) {
			result = replace;
		}
		if (result == VoxelType::WATER)
			result = replace;

		/*if (!atGround && aboveGround && distanceFromSeaLevel<SEA_HEIGHT)/2
			result = Voxel::SAND;*/
	}

	return result;
}
