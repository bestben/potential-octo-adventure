#include "ChunkIOManager.h"

#include <sstream>
#include <iostream>

#include "chunk.h"

ChunkIOManager::ChunkIOManager( int iSeed ) {
	m_bCanSave = true;

	std::ostringstream filename;
	filename << "world/" << iSeed << ".chunkMap";
	std::ostringstream filename2;
	filename2 << "world/" << iSeed << ".chunk";

	int iFlag = 0;
	std::fstream tmpFile(filename.str(), std::fstream::in | std::fstream::binary);
	if (tmpFile.fail())
	{
		tmpFile.close();
		tmpFile.open(filename.str(), std::fstream::out | std::fstream::binary | std::fstream::trunc);
		tmpFile << 0xABCDEFAB;
		tmpFile.close();
		tmpFile.open(filename2.str(), std::fstream::out | std::fstream::binary | std::fstream::trunc);
		tmpFile << 0xABCDEFAB;
		tmpFile.close();
		iFlag = std::fstream::trunc;
	}

	m_oChunkMappingFile.open(filename.str(), std::fstream::in | std::fstream::out | std::fstream::binary | iFlag);
	if (m_oChunkMappingFile.fail())
	{
		m_bCanSave = false;
		return;
	}
	m_oChunkDataFile.open(filename2.str(), std::fstream::in | std::fstream::out | std::fstream::binary | iFlag);
	if (m_oChunkDataFile.fail() )
	{
		m_bCanSave = false;
		return;
	}

	m_oChunkMappingFile.seekp(0, std::ios_base::end);
	int iSize = (int)m_oChunkMappingFile.tellg();
	m_oChunkMappingFile.seekp(0, std::ios_base::beg);

	for (int i = 0; i < iSize / (4 * 4); ++i) {
		Coords c;
		int offset;
		m_oChunkMappingFile.read((char*)&c, sizeof(Coords));
		m_oChunkMappingFile.read((char*)&offset, sizeof(int));
		m_oChunkMapping[c] = offset;
	}
}

ChunkIOManager::~ChunkIOManager() {
	m_oChunkMappingFile.close();
	m_oChunkDataFile.close();
}

void ChunkIOManager::loadChunk(const Coords& oChunkCoord, Voxel* pData) {
	if (!m_bCanSave)
		return;
	MI_ASSERT( pData != NULL );

	memset(pData, 0, sizeof(Voxel)*CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE);

	std::map<Coords, int>::iterator it = m_oChunkMapping.find(oChunkCoord);
	if (it == m_oChunkMapping.end()) {
		return;
	}

	m_oChunkDataFile.seekg(it->second);
	m_oChunkDataFile.read((char*)pData, sizeof(Voxel) * CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE);
}

void ChunkIOManager::saveChunk(const Coords& oChunkCoord, Voxel* pData) {
	if (!m_bCanSave)
		return;
	std::map<Coords, int>::iterator it = m_oChunkMapping.find(oChunkCoord);
	if (it == m_oChunkMapping.end()) {
		m_oChunkDataFile.seekp(0, std::ios_base::end);
		m_oChunkMappingFile.seekp(0, std::ios_base::end);

		int iOffset = (int)m_oChunkDataFile.tellg();

		m_oChunkMappingFile.write((char*)&oChunkCoord, sizeof(Coords));
		m_oChunkMappingFile.write((char*)&iOffset, sizeof(int));
		m_oChunkMapping[oChunkCoord] = iOffset;
	} else {
		m_oChunkDataFile.seekp(it->second);
	}

	m_oChunkDataFile.write((char*)pData, sizeof(Voxel) * CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE);
}

bool ChunkIOManager::chunkExist(const Coords& oChunkCoord) {
	if (!m_bCanSave)
		return false;
	std::map<Coords, int>::iterator it = m_oChunkMapping.find(oChunkCoord);

	return it != m_oChunkMapping.end();
}
