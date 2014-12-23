#pragma once

#include "defs.h"

#define BIOMESHIFT 9
#define BIOMESIZE 512


class BiomeLayer
{
public:
	BiomeLayer();
	~BiomeLayer();

	void generate(double amplitude, double offset, double scale, double xOffset, double yOffset, long seed);
	void setClamp(double min, double max);

	bool isReady();

	void outputDebugFile(char *filename);

private:

	double clamp(double value);

	double mData[BIOMESIZE*BIOMESIZE];
	long mSeed;
	double mClampMin;
	double mClampMax;
	bool mHasClamp = false;
	bool mReady = false;
};



