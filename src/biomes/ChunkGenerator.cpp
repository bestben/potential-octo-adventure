#include "ChunkGenerator.h"
#include "BiomeMap.h"
#include "../libnoise/noise.h"
#include <fstream>
#include <iostream>
#include <random>
#include <set>

ChunkGenerator::ChunkGenerator(ChunkManager* cm, int worldSeed) : mChunkManager{ cm }, mMaps(), mLog("log.txt", std::ios_base::out), mWorldSeed(worldSeed)
{

}


ChunkGenerator::~ChunkGenerator()
{
    for (auto& pair : mMaps) {
        delete pair.second;
    }
    mMaps.clear();
	mLog.close();
}

bool ChunkGenerator::generateChunk(Voxel* data, int i, int j, int k, std::set<Coords> &modifiedChunks) {
	Coords chunkId = { i, j, k };
	return generateChunk(data, chunkId, modifiedChunks);
}

bool ChunkGenerator::generateChunk(Voxel* data, Coords chunkId, std::set<Coords> &modifiedChunks) {
	//mLog << "Generating chunk [" << chunkId.i << "," << chunkId.j << "," << chunkId.k << "]" << std::endl;

	modifiedChunks.insert(chunkId);
	Coords mapCoords = GetChunkBiomeMap(chunkId);

    auto it = mMaps.find(mapCoords);
    if (it == mMaps.end()) {
		mMaps[mapCoords] = new BiomeMap(mapCoords.i, mapCoords.k, mWorldSeed);
        it = mMaps.find(mapCoords);
	}

	bool onlyAir = true;

    //memset(data, 0, CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE*sizeof(Voxel));

	for (int z = 0; z < CHUNK_SIZE; ++z) {
		for (int y = 0; y < CHUNK_SIZE; ++y) { // Hauteur
			for (int x = 0; x < CHUNK_SIZE; ++x) {
				int index = IndexVoxelRelPos({ x, y, z });
                VoxelType type = it->second->getVoxelType(chunkId, x, y, z);
				data[index].type = type;
				if (onlyAir && type != VoxelType::AIR)
					onlyAir = false;
				//data[index]._light = SUN_LIGHT;
			}
		}
	}
    bool notrees = placeTrees(data, chunkId, *(it->second), modifiedChunks);
    onlyAir = onlyAir && notrees;

	return onlyAir;
}


static int rndInt(std::minstd_rand0 &gen, int min, int max){

	return gen() % (max - min) + min;
}


bool ChunkGenerator::placeTrees(Voxel* /*data*/, Coords chunkId, BiomeMap &map, std::set<Coords> &modifiedChunks){


	std::minstd_rand0 rnd((chunkId.i*773 + chunkId.j*104743 + chunkId.k*15485867)*mWorldSeed);

	Coords chunkPos = chunkId * CHUNK_SIZE;

        unsigned int count = 3;

	
	Voxel trunk = getVoxelFromType(VoxelType::TRUNK);
	Voxel leaves = getVoxelFromType(VoxelType::LEAVES);

	bool onlyAir = true;

        for(unsigned int i=0 ; i<count ; ++i){

                unsigned int height = rndInt(rnd, 4, 10);

		Coords min{ -3, -1, -3 };
		Coords max{ 3, 3, 3 };

		min -= (height / 3)-1;
		max += (height / 3)-1;

		Coords startPos = Coords{rndInt(rnd, 0 - min.i, CHUNK_SIZE - max.i), 0, rndInt(rnd, 0 - min.k, CHUNK_SIZE - max.k)};

		int ground = map.getGroundLevel(chunkId, startPos.i, startPos.k);


		if (!(ground >= chunkPos.j && ground<chunkPos.j + CHUNK_SIZE)) {
			break;
		}

		startPos.j = ground - chunkPos.j + 1;


		Voxel below = mChunkManager->getVoxel(chunkPos + startPos);
		Voxel startVoxel = mChunkManager->getVoxel(chunkPos + startPos + Coords{ 0, 1, 0 });

		if (below.type != VoxelType::GRASS || startVoxel.type != VoxelType::AIR)
			break;

		onlyAir = false;

                for(unsigned int h=0 ; h<height ; ++h){
			startPos += {0,1,0};
			
			Coords pos = chunkPos + startPos;
			mChunkManager->setVoxel(pos, VoxelType::TRUNK);
			modifiedChunks.insert(GetChunkPosFromVoxelPos(pos));
		}




        //Coords size{ rnd()%3 + 3, rnd()%2 + 2, rnd()%3 + 3 };
		{
			for (int z = -1; z <= 1; ++z)
			for (int y = -1; y <= 1; ++y)
			for (int x = -1; x <= 1; ++x){
				Voxel before = mChunkManager->getVoxel(chunkPos + startPos + Coords{ x, y, z });
				if (before.type == VoxelType::AIR || before.type == VoxelType::IGNORE_TYPE){
					Coords pos = chunkPos + startPos + Coords{ x, y, z };
					mChunkManager->setVoxel(pos, VoxelType::LEAVES);
					modifiedChunks.insert(GetChunkPosFromVoxelPos(pos));
				}
			}
		}	
		
                for(unsigned int c=0 ; c<height*2 ; ++c){

			int d = 1;
			if (height >= 7)
				d = 2;
			else if (height >= 9)
				d = 3;
			Coords p{ rndInt(rnd, min.i, max.i-1), rndInt(rnd, min.j, max.j-1),rndInt(rnd, min.k, max.k-1) };


			for (int z = 0; z <= d; ++z)
			for (int y = 0; y <= d; ++y)
			for (int x = 0; x <= d; ++x){

				Voxel before = mChunkManager->getVoxel(chunkPos + p + startPos + Coords{ x, y, z });
				if (before.type == VoxelType::AIR || before.type == VoxelType::IGNORE_TYPE){
					Coords pos = chunkPos + startPos + p + Coords{ x, y, z };
					mChunkManager->setVoxel(pos, VoxelType::LEAVES);
					modifiedChunks.insert(GetChunkPosFromVoxelPos(pos));
				}
			}

		}
	}
	
	return onlyAir;
}
