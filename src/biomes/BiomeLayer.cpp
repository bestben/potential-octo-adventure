#include "BiomeLayer.h"
#include "OpenSimplexNoise.hpp"
#include <fstream>
#include <iostream>




BiomeLayer::BiomeLayer()
{
	mData = new double[CHUNK_SIZE*CHUNK_SIZE];
}


BiomeLayer::~BiomeLayer()
{
	delete mData;
}


void BiomeLayer::generate(double amplitude, double offset, double scale, double xOffset, double yOffset, long seed){
	mSeed = seed;

	OpenSimplexNoise noiseA(mSeed);

	for (int x = 0; x < CHUNK_SIZE; ++x){
		for (int y = 0; y < CHUNK_SIZE; ++y){
			mData[(y*CHUNK_SIZE) + x] = clamp(offset + amplitude*(noiseA.value(((double)x + xOffset)*scale, ((double)y + yOffset)*scale) + 1.0) / 2.0);
		}
	}
	mReady = true;
	
}


void BiomeLayer::setClamp(double min, double max){
	mClampMin = std::min(min,max);
	mClampMax = std::max(min, max);
	mHasClamp = true;
}

double BiomeLayer::clamp(double value) const{
	return mHasClamp ? (value > mClampMax ? mClampMax : (value < mClampMin ? mClampMin : value)) : value;
}

bool BiomeLayer::isReady() const{
	return mReady;
}

double BiomeLayer::getValue(int i, int k) const{
	return mData[k*CHUNK_SIZE + i];
}


void BiomeLayer::outputDebugFile(const char *filename) const{

	short *data = new short[CHUNK_SIZE*CHUNK_SIZE];

	short min = (1<<15) - 1;
	short max = (1<<15);
	for (int i = 0; i < CHUNK_SIZE*CHUNK_SIZE; ++i){
		data[i] = static_cast<short>(mData[i]);
		if (data[i] < min) min = data[i];
		if (data[i] > max) max = data[i];
	}

	// TODO: Remove debug
	std::cout << min << " - " << max << std::endl;

	std::ofstream file;
	file.open(filename, std::ios::basic_ios::out | std::ios::basic_ios::binary);
	file.write((const char *)data, CHUNK_SIZE*CHUNK_SIZE*sizeof(short));
	file.close();

	delete data;
}