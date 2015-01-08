#pragma once


#include "chunk.h"
#include <fstream>
#include <sstream>


void SaveChunkToDisk(Voxel *data, const Coords &chunkId, bool onlyAir);

bool LoadChunkFromDisk(Voxel *data, const Coords &chunkId, bool *onlyAir);

inline bool FileExists(const std::string& filename){
	std::ifstream file(filename);
	return file.good();;
}

inline bool ChunkExistsOnDisk(const Coords &chunkId){
	std::ostringstream filename;
	filename << "world/" << chunkId.i << "-" << chunkId.j << "-" << chunkId.k;
	return FileExists(filename.str());
}
