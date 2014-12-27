#pragma once

#include "../chunk.h"
#include "OpenSimplexNoise.hpp"


class BiomeLayer
{
public:
	BiomeLayer(double amplitude, double offset, double scale, double xOffset, double yOffset, long seed);
	~BiomeLayer();

	double getValue(Coords chunkId, int i, int k);

private:
	OpenSimplexNoise mNoise;

	double mAmplitude;
	double mOffset;
	double mScale;
	double mXOffset;
	double mYOffset;
	long mSeed;

	double *mData;
};



