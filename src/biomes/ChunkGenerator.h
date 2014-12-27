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
	void generateChunk(Voxel* data, int i, int j, int k);
	void generateChunk(Voxel* data, Coords chunkId);

private:
	BiomeMap mMap;
	std::ofstream mLog;

};

