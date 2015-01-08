#pragma once
#include "chunk.h"
#include <QtCore/QQueue>

#define MAX_LIGHT_UPDATES_PER_FRAME 1024*32


class GameWindow;
class ChunkManager;


class LightManager
{
public:
	LightManager(ChunkManager* cm);
	~LightManager();
	

	void placeVoxel(Coords pos, VoxelType type, QSet<Coords> &modifiedChunks);
	void removeVoxel(Coords pos, QSet<Coords> &modifiedChunks);

	void updateLighting(Chunk* chunk);

	void lightNeighbors(Coords coords, QSet<Coords> &modifiedChunks);
	void unlightNeighbors(Coords coords, uint8 old_light, QSet<Coords> &light_sources, QSet<Coords> &modifiedChunks);
	
	bool propagateSunLight(Chunk* chunk, QSet<Coords>& lightSources, QSet<Coords> &modifiedChunks);
	uint16 propagateSunLight(Coords start, QSet<Coords> &modifiedChunks);

	
	void spreadLight(QSet<Coords>& lightSources, QSet<Coords> &modifiedChunks);
	void unspreadLight(QHash<Coords, uint8> &from, QSet<Coords> &lightSources, QSet<Coords> &modifiedChunks);

private:
	ChunkManager* mChunkManager;
	
};

