#include "ChunkGenerator.h"
#include "BiomeMap.h"
#include <QtCore/QTime>
#include <iostream>
#include <fstream>
#include "omp.h"

ChunkGenerator::ChunkGenerator() : mMaps()
{

	Voxel** data = new Voxel*[5*7*5];
	for (int i = 0; i < 5 * 5 * 7; ++i)
		data[i] = new Voxel[CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE];
	

	QTime t;
	t.start();

	#pragma omp parallel for
	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 7; j++) {
			for (int k = 0; k < 5; k++) {
				generateChunk(data[i*5*7 + j*5 + k], i, j, k);
			}
		}
	}

	std::ofstream log("log.txt");
	log << "Generation en " << t.elapsed() << "ms" << std::endl;
	log.close();

	//delete data;
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

	
	for (int x = 0; x < CHUNK_SIZE; ++x) {
		for (int y = 0; y < CHUNK_SIZE; ++y) { // Hauteur
			int currentHeight = y + j*CHUNK_SIZE;
			for (int z = 0; z < CHUNK_SIZE; ++z) {
				data[z*CHUNK_SIZE*CHUNK_SIZE + y*CHUNK_SIZE + x] = (Voxel) map->getVoxelType(x,currentHeight,z);
			}

		}

	}




}