#pragma once

#include "../chunk.h"
#include <memory>
#include <fstream>
#include "BiomeMap.h"
#include "../chunkmanager.h"
#include <unordered_map>

class ChunkManager;
 /*
 * @brief Classe permettant de générer un chunk.
 */
class ChunkGenerator {
public:
	ChunkGenerator(ChunkManager *cm, int worldSeed = 0);
	~ChunkGenerator();
	bool generateChunk(Voxel* data, int i, int j, int k, QSet<Coords> &modifiedChunks);
	bool generateChunk(Voxel* data, Coords chunkId, QSet<Coords> &modifiedChunks);

private:
    std::unordered_map<Coords, BiomeMap*> mMaps;
	ChunkManager* mChunkManager;
	bool placeTrees(Voxel* data, Coords chunkId, BiomeMap& map, QSet<Coords> &modifiedChunks);
	std::ofstream mLog;
	int mWorldSeed;
};

