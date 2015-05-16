#pragma once

#include <memory>
#include <fstream>
#include <unordered_map>
#include <set>

#include "../chunk.h"
#include "BiomeMap.h"
#include "../chunkmanager.h"


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

