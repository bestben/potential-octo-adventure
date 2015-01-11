#pragma once

#include "../chunk.h"
#include <memory>
#include <fstream>
#include "BiomeMap.h"
#include <unordered_map>

/**
 * @brief Classe permettant de générer un chunk.
 */
class ChunkGenerator {
public:
	ChunkGenerator();
	~ChunkGenerator();
	bool generateChunk(Voxel* data, int i, int j, int k);
	bool generateChunk(Voxel* data, Coords chunkId);

private:
    std::unordered_map<Coords, BiomeMap*> mMaps;
	std::ofstream mLog;

};

