#pragma once

#include "../chunk.h"
#include <memory>
#include <fstream>
#include "BiomeMap.h"
#include "../chunkmanager.h"
#include <unordered_map>
#include <set>

class ChunkManager;
 /*
 * @brief Classe permettant de générer un chunk.
 */
class ChunkGenerator {
public:
	ChunkGenerator(ChunkManager *cm, int worldSeed = 0);
	~ChunkGenerator();
	bool generateChunk(Voxel* data, int i, int j, int k, std::set<Coords> &modifiedChunks);
	bool generateChunk(Voxel* data, Coords chunkId, std::set<Coords> &modifiedChunks);

private:
    std::unordered_map<Coords, BiomeMap*> mMaps;
	ChunkManager* mChunkManager;
	bool placeTrees(Voxel* data, Coords chunkId, BiomeMap& map, std::set<Coords> &modifiedChunks);
	std::ofstream mLog;
	int mWorldSeed;
};

