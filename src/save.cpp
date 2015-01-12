#include "save.h"
#include <string>


void SaveChunkToDisk(Voxel* data, const Coords &chunkId, bool onlyAir, int worldSeed){

	uint32 size = CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE;

	

	std::ostringstream filename;
	filename << "world/" << worldSeed << "-" << chunkId.i << "-" << chunkId.j << "-" << chunkId.k << ".chunk";

	std::ofstream file;
	file.open(filename.str(), std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);


	if(onlyAir){
		file.write((char*)&onlyAir, sizeof(bool));
		file.close();
		return;
	}else{
		file.write((char*)&onlyAir, sizeof(bool));
	}

	uint8 *tmp_data = new uint8[size];

	for(uint32 i=0 ; i<size ;++i){
		tmp_data[i] = (uint8)data[i].type;
	}

	file.write((char *)tmp_data, size*sizeof(uint8));

	file.close();
	file.clear();

	delete tmp_data;

}

bool LoadChunkFromDisk(Voxel *data, const Coords &chunkId, bool *onlyAir, int worldSeed){

	std::ostringstream filename;
	filename << "world/" << worldSeed << "-" << chunkId.i << "-" << chunkId.j << "-" << chunkId.k << ".chunk";

	std::ifstream file;
	file.open(filename.str(), std::ios_base::in | std::ios_base::binary);

	if(!file.good()){
		return false;
	}
	uint32 size = CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE;

	file.read((char*)onlyAir, sizeof(bool));

	if(onlyAir){
		if(*onlyAir){
			file.close();
			for (uint32 i = 0; i < size; ++i){
				data[i] = Voxel(VoxelType::AIR);
				data[i]._light = 0;
			}
			return true;
		}
	}

	uint8 *tmp_data = new uint8[size];

	file.read((char *)tmp_data, size*sizeof(uint8));

	for(uint32 i=0; i<size;++i){
		data[i] = Voxel( (VoxelType)tmp_data[i] );
		data[i]._light = 0;
	}

	file.close();
	delete tmp_data;

	return true;
}