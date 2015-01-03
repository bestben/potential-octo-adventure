#pragma once
#include "../chunk.h"
#include <memory>
#include <fstream>
#include "BiomeMap.h"


class ChunkGenerator
{
public:
	ChunkGenerator();
	~ChunkGenerator();
	bool generateChunk(Voxel* data, int i, int j, int k);
	bool generateChunk(Voxel* data, Coords chunkId);

private:
	QHash<Coords, BiomeMap*> mMaps;
	std::ofstream mLog;

};

