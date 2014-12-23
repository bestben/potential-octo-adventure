#include "BiomeLayer.h"
#include "OpenSimplexNoise.hpp"
#include <fstream>
#include <iostream>




BiomeLayer::BiomeLayer()
{
}


BiomeLayer::~BiomeLayer()
{
}


void BiomeLayer::generate(double amplitude, double offset, double scale, double xOffset, double yOffset, long seed){
	mSeed = seed;

	OpenSimplexNoise noiseA(mSeed);

	for (int x = 0; x < BIOMESIZE; ++x){
		for (int y = 0; y < BIOMESIZE; ++y){
			mData[(y << BIOMESHIFT) + x] = clamp(offset + amplitude*(noiseA.value(((double)x + xOffset)*scale, ((double)y + yOffset)*scale) + 1.0) / 2.0);
		}
	}
	mReady = true;
	
}


void BiomeLayer::setClamp(double min, double max){
	mClampMin = std::min(min,max);
	mClampMax = std::max(min, max);
	mHasClamp = true;
}

double BiomeLayer::clamp(double value){
	return mHasClamp ? (value > mClampMax ? mClampMax : (value < mClampMin ? mClampMin : value)) : value;
}

bool BiomeLayer::isReady(){
	return mReady;
}


void BiomeLayer::outputDebugFile(char *filename){

	short *data = new short[BIOMESIZE*BIOMESIZE];

	short min = (1<<15) - 1;
	short max = (1<<15);
	for (int i = 0; i < BIOMESIZE*BIOMESIZE; ++i){
		data[i] = static_cast<short>(mData[i]);
		if (data[i] < min) min = data[i];
		if (data[i] > max) max = data[i];
	}

	// TODO: Remove debug
	std::cout << min << " - " << max << std::endl;

	std::ofstream file;
	file.open(filename, std::ios::basic_ios::out | std::ios::basic_ios::binary);
	file.write((const char *)data, BIOMESIZE*BIOMESIZE*sizeof(short));
	file.close();

	delete data;
}