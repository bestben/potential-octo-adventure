#pragma once
#include "chunk.h"
#include <QtCore/QQueue>

#define MAX_LIGHT_UPDATES_PER_FRAME 1024*32


class GameWindow;
class ChunkManager;

#if 0
struct LightNode
{
	Coords pos;
	Chunk* chunk;
};

struct LightRemovalNode
{
	Coords pos;
	Chunk* chunk;
	uint8 value;
};
#endif

class LightManager
{
public:
	LightManager(ChunkManager* cm);
	~LightManager();
	

	void placeVoxel(Coords pos, VoxelType type, QSet<Coords> &modifiedChunks);
	void removeVoxel(Coords pos, QSet<Coords> &modifiedChunks);

	void updateLighting(Chunk* chunk);

	void lightNeighbors(Coords coords, QSet<Coords> &modifiedChunks);
	void unlightNeighbors(Coords coords, uint8 old_light, QSet<Coords> light_sources, QSet<Coords> &modifiedChunks);
	
	bool propagateSunLight(Chunk* chunk, QSet<Coords>& lightSources, QSet<Coords> &modifiedChunks);
	uint16 propagateSunLight(Coords start, QSet<Coords> &modifiedChunks);

	
	void spreadLight(QSet<Coords>& lightSources, QSet<Coords> &modifiedChunks);
	void unspreadLight(QHash<Coords, uint8> &from, QSet<Coords> &lightSources, QSet<Coords> &modifiedChunks);



#if 0

	void processChunk(Chunk* chunk);
	void initializeSunlight(Chunk* chunk);
	void placeTorchLight(Coords voxelCoords, uint8 amount);
	void voxelChanged(Coords voxelCoords);



	void removeTorchLight(Coords voxelCoords);
	void update(GameWindow* gl);
	void processNodeNeighbor(LightNode& node, Coords dir, uint8 light, Voxel* currentData);
	void processNodeRemovalNeighbor(LightRemovalNode& node, Coords dir, Voxel* currentData);
	void processSunNodeNeighbor(LightNode& node, Coords dir, uint8 light, Voxel* currentData);
	void processSunNodeRemovalNeighbor(LightRemovalNode& node, Coords dir, Voxel* currentData);

#endif

private:
	ChunkManager* mChunkManager;
#if 0
	QQueue<LightNode> torchLightQueue;
	QQueue<LightRemovalNode> torchLightRemovalQueue;

	QQueue<LightNode> sunLightQueue;
	QQueue<LightRemovalNode> sunLightRemovalQueue;

	QHash<Coords, QQueue<LightNode>> torchLightUnloadedQueues;
	QHash<Coords, QQueue<LightRemovalNode>> torchLightRemovalUnloadedQueues;

	QHash<Coords, QQueue<LightNode>> sunLightUnloadedQueues;
	QHash<Coords, QQueue<LightRemovalNode>> sunLightRemovalUnloadedQueues;

	QQueue<Chunk*> dirtyLightMaps;
#endif
	
};

