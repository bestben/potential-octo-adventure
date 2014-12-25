#include "ChunkGenerator.h"
#include "BiomeMap.h"
#include <fstream>
#include <iostream>

ChunkGenerator::ChunkGenerator() : mMaps(), mLog("log.txt", std::ios_base::out)
{

}


ChunkGenerator::~ChunkGenerator()
{
	mLog.close();
}

void ChunkGenerator::generateChunk(Voxel* data, int i, int j, int k) {
	// mLog << "Generating chunk [" << i << "," << j << "," << k << "]" << std::endl;
	
	MapIndex index;
	index.a = i;
	index.b = k;

	// On cache les maps générées pour le futur
	// TODO: Eviter de garder trop de maps en cache

	BiomeMap *map;
	if (mMaps.contains(index)) {
		map = mMaps[index].get();
	} else {
		map = new BiomeMap(index.a, index.b);
		mMaps.insert(index, std::shared_ptr<BiomeMap>(map));
	}

	for (int x = 0; x < CHUNK_SIZE; ++x) {
		for (int y = 0; y < CHUNK_SIZE; ++y) { // Hauteur
			int currentHeight = y + j*CHUNK_SIZE;
			for (int z = 0; z < CHUNK_SIZE; ++z) {
				data[z*CHUNK_SIZE*CHUNK_SIZE + y*CHUNK_SIZE + x] = map->getVoxelType(x,currentHeight,z);
			}

		}

	}




}