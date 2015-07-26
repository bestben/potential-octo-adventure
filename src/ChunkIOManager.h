#pragma once

#include <map>
#include <fstream>
#include "coords.h"

class ChunkIOManager {
public:

	ChunkIOManager( int iSeed );
	~ChunkIOManager();

	void	loadChunk(const Coords& oChunkCoord, Voxel* oData);
	void	saveChunk(const Coords& oChunkCoord, Voxel* oData);
	bool	chunkExist( const Coords& oChunkCoord );

private:
	std::map<Coords, int>	m_oChunkMapping;

	std::fstream			m_oChunkMappingFile;
	std::fstream			m_oChunkDataFile;

	bool					m_bCanSave;
};