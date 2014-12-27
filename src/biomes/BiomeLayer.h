#pragma once

#include "../chunk.h"
#include "OpenSimplexNoise.hpp"


class BiomeLayer
{
public:
	BiomeLayer(double amplitude, double offset, double scale, double xOffset, double yOffset, long seed);
	~BiomeLayer();

	void setClamp(double min, double max);
	double getValue(Coords chunkId, int i, int k);

private:

	double clampLayer(double value) const;

	OpenSimplexNoise mNoise;

	double mAmplitude;
	double mOffset;
	double mScale;
	double mXOffset;
	double mYOffset;
	long mSeed;

	double mClampMin;
	double mClampMax;
	bool mHasClamp = false;
};



