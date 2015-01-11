#include "ChunkGenerator.h"
#include "BiomeMap.h"
#include <fstream>
#include <iostream>

ChunkGenerator::ChunkGenerator() : mMaps(), mLog("log.txt", std::ios_base::out)
{

}


ChunkGenerator::~ChunkGenerator()
{
    for (auto& pair : mMaps) {
        delete pair.second;
	}
	mLog.close();
}

bool ChunkGenerator::generateChunk(Voxel* data, int i, int j, int k) {
	Coords chunkId = { i, j, k };
	return generateChunk(data, chunkId);
}

bool ChunkGenerator::generateChunk(Voxel* data, Coords chunkId) {
	//mLog << "Generating chunk [" << chunkId.i << "," << chunkId.j << "," << chunkId.k << "]" << std::endl;

	Coords mapCoords = GetChunkBiomeMap(chunkId);

    auto it = mMaps.find(mapCoords);
    if (it == mMaps.end()) {
		mMaps[mapCoords] = new BiomeMap(mapCoords.i, mapCoords.k);
        it = mMaps.find(mapCoords);
	}/* else {
		mLog << "reusing cached map : " << mapCoords.i << " - " << mapCoords.k << std::endl;
	}*/
	bool onlyAir = true;
	for (int z = 0; z < CHUNK_SIZE; ++z) {
		for (int y = 0; y < CHUNK_SIZE; ++y) { // Hauteur
			for (int x = 0; x < CHUNK_SIZE; ++x) {
				int index = IndexVoxelRelPos({ x, y, z });
                VoxelType type = it->second->getVoxelType(chunkId, x, y, z);
				data[index].type = type;
				if (onlyAir && type != VoxelType::AIR)
					onlyAir = false;
				//data[index].sunLight = 15;
			}
		}
	}

	return onlyAir;
}
