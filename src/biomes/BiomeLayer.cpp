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
}


BiomeLayer::~BiomeLayer()
{
}



void BiomeLayer::setClamp(double min, double max){
	mClampMin = std::min(min,max);
	mClampMax = std::max(min, max);
	mHasClamp = true;
}

double BiomeLayer::clampLayer(double value) const{
	return mHasClamp ? clamp(value, mClampMin, mClampMax) : value;
}


double BiomeLayer::getValue(Coords chunkId, int i, int k){

	int x = i + chunkId.i*CHUNK_SIZE;
	int z = k + chunkId.k*CHUNK_SIZE;

	double noiseValue = mNoise.value(((double)x + mXOffset)*mScale, ((double)z + mYOffset)*mScale);
	return clampLayer(mOffset + mAmplitude*(noiseValue + 1.0)*0.5);
}

