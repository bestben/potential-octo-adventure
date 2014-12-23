#pragma once

#include <unordered_set>

class BiomeLayer;

class BiomeMap
{
public:
	BiomeMap();
	~BiomeMap();

private:
	std::unordered_set < std::string, BiomeLayer* > mLayers;
};

