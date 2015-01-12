#pragma once


#include "chunk.h"
#include <fstream>
#include <sstream>


void SaveChunkToDisk(Voxel *data, const Coords &chunkId, bool onlyAir, int worldSeed);

bool LoadChunkFromDisk(Voxel *data, const Coords &chunkId, bool *onlyAir, int worldSeed);

inline bool FileExists(const std::string& filename){
	std::ifstream file(filename);
	return file.good();;
}

inline bool ChunkExistsOnDisk(const Coords &chunkId, int worldSeed){
	std::ostringstream filename;
	filename << "world/" << worldSeed << "-" << chunkId.i << "-" << chunkId.j << "-" << chunkId.k << ".chunk";
	return FileExists(filename.str());
}
