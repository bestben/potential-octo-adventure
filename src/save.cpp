#include "save.h"
#include <string>


void SaveChunkToDisk(Voxel* data, const Coords &chunkId, bool onlyAir){

	uint32 size = CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE;

	

	std::ostringstream filename;
	filename << "world/" << chunkId.i << "-" << chunkId.j << "-" << chunkId.k;

	std::ofstream file;
	file.open(filename.str(), std::ofstream::out | std::ofstream::binary);

	if(onlyAir){
		file.write((char*)&onlyAir, sizeof(bool));
		file.close();
		return;
	}else{
		file.write((char*)&onlyAir, sizeof(bool));
	}

	uint8 *tmp_data = new uint8[size];

	for(uint32 i=0 ; i<size ;++i){
		tmp_data[i] = (uint32)data[i].type;
	}

	file.write((char *)tmp_data, size*sizeof(uint8));

	file.close();

	delete tmp_data;

}

bool LoadChunkFromDisk(Voxel *data, const Coords &chunkId, bool *onlyAir){

	std::ostringstream filename;
	filename << "world/" << chunkId.i << "-" << chunkId.j << "-" << chunkId.k;

	std::ifstream file;
	file.open(filename.str(), std::ifstream::in | std::ifstream::binary);

	if(!file.good()){
		return false;
	}
	uint32 size = CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE;

	file.read((char*)onlyAir, sizeof(bool));

	if(onlyAir){
		if(*onlyAir){
			memset(data, 0, size*sizeof(Voxel));
			file.close();
			return true;
		}
	}

	uint8 *tmp_data = new uint8[size];

	file.read((char *)tmp_data, size*sizeof(uint8));

	for(uint32 i=0; i<size;++i){
		data[i] = Voxel( (VoxelType)tmp_data[i] );
	}

	file.close();
	delete tmp_data;

	return true;
}