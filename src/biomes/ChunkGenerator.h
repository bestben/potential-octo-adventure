#pragma once
#include "../chunk.h"
#include <memory>
#include <fstream>
#include "BiomeMap.h"
#include "../chunkmanager.h"

class ChunkManager;

class ChunkGenerator
{
public:
	ChunkGenerator(ChunkManager *cm);
	~ChunkGenerator();
	bool generateChunk(Voxel* data, int i, int j, int k, QSet<Coords> &modifiedChunks);
	bool generateChunk(Voxel* data, Coords chunkId, QSet<Coords> &modifiedChunks);

private:
	ChunkManager* mChunkManager;
	bool placeTrees(Voxel* data, Coords chunkId, BiomeMap& map, QSet<Coords> &modifiedChunks);
	QHash<Coords, BiomeMap*> mMaps;
	std::ofstream mLog;

};

