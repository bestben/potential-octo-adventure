#pragma once
#include "../chunk.h"
#include <memory>
#include <fstream>

class BiomeMap;

class ChunkGenerator
{
public:
	ChunkGenerator();
	~ChunkGenerator();
	void generateChunk(Voxel* data, int i, int j, int k);

private:
	QHash<MapIndex, std::shared_ptr<BiomeMap>> mMaps;
	std::ofstream mLog;

};

