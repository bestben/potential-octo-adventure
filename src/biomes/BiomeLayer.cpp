#include "BiomeLayer.h"
#include "OpenSimplexNoise.hpp"
#include "../chunk.h"



BiomeLayer::BiomeLayer(double amplitude, double offset, double scale, double xOffset, double yOffset, long seed) :
mAmplitude(amplitude),
mOffset(offset),
mScale(scale),
mXOffset(xOffset),
mYOffset(yOffset),
mSeed(seed),
mNoise(seed)
{
	mData = new double[BIOMEMAP_SIZE*BIOMEMAP_SIZE];
	for (int z = 0; z < BIOMEMAP_SIZE; z++) {
		for (int x = 0; x < BIOMEMAP_SIZE; x++) {
			double noiseValue = mNoise.value(((double)x + mXOffset)*mScale, ((double)z + mYOffset)*mScale);
			noiseValue = mOffset + mAmplitude*(noiseValue + 1.0)*0.5;
			mData[z*BIOMEMAP_SIZE + x] = noiseValue;
		}
	}
}


BiomeLayer::~BiomeLayer()
{
	delete mData;
}


double BiomeLayer::getValue(Coords chunkIdInMap, int i, int k){

	Assert(i >= 0 && i<CHUNK_SIZE && k >= 0 && k<CHUNK_SIZE && chunkIdInMap.i >= 0 && chunkIdInMap.i<BIOMEMAP_CHUNKS && chunkIdInMap.k >= 0 && chunkIdInMap.k<BIOMEMAP_CHUNKS)

	int x = i + chunkIdInMap.i*CHUNK_SIZE;
	int z = k + chunkIdInMap.k*CHUNK_SIZE;
	
	return mData[z*BIOMEMAP_SIZE + x];
}

