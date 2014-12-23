#include "ChunkGenerator.h"
#include "BiomeMap.h"
#include <QtCore/QTime>
#include <iostream>
#include <fstream>

ChunkGenerator::ChunkGenerator() : mMaps()
{
	Voxel* data = new Voxel[CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE];

	QTime t;
	t.start();
	for (int i = -2; i < 3; i++) {
		for (int j = 0; j < 7; j++) {
			for (int k = -2; k < 3; k++) {
				generateChunk(data, i, j, k);
			}
		}
	}

	std::ofstream log("log.txt");
	log << "Generation en " << t.elapsed() << "ms" << std::endl;
	log.close();

	delete data;
}


ChunkGenerator::~ChunkGenerator()
{
}

void ChunkGenerator::generateChunk(Voxel* data, int i, int j, int k) {
	MapIndex index;
	index.a = i;
	index.b = k;

	BiomeMap *map;
	if (mMaps.contains(index)) {
		map = mMaps[index].get();
	} else {
		map = new BiomeMap(index.a, index.b);
		mMaps.insert(index, std::shared_ptr<BiomeMap>(map));
	}

	for (int z = 0; z < CHUNK_SIZE; ++z) {
		for (int x = 0; x < CHUNK_SIZE; ++x) {
			for (int y = 0; y < CHUNK_SIZE; ++y) { // Hauteur
				int currentHeight = y + j*CHUNK_SIZE;
				data[z*CHUNK_SIZE*CHUNK_SIZE + y*CHUNK_SIZE + x] = (Voxel) map->getVoxelType(x,y,currentHeight);
			}

		}

	}




}