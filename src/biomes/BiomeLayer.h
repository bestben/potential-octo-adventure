#pragma once

#include "../defs.h"




class BiomeLayer
{
public:
	BiomeLayer();
	~BiomeLayer();

	void generate(double amplitude, double offset, double scale, double xOffset, double yOffset, long seed);
	void setClamp(double min, double max);
	bool isReady() const;
	double getValue(int i, int k) const;


	void outputDebugFile(const char *filename) const;

private:

	double clamp(double value) const;

	double *mData;
	long mSeed;
	double mClampMin;
	double mClampMax;
	bool mHasClamp = false;
	bool mReady = false;
};



