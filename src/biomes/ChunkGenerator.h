#pragma once
#include "../defs.h"
#include <memory>

class BiomeMap;

class ChunkGenerator
{
public:
	ChunkGenerator();
	~ChunkGenerator();
	void generateChunk(Voxel* data, int i, int j, int k);

private:
	QHash<MapIndex, std::shared_ptr<BiomeMap>> mMaps;

};

