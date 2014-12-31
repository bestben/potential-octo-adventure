#pragma once
#include "chunk.h"
#include <QtCore/QQueue>

#define MAX_LIGHT_UPDATES_PER_FRAME 1024*32
class GameWindow;
class ChunkManager;

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

class LightManager
{
public:
	LightManager(ChunkManager* cm);
	~LightManager();
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
private:
	ChunkManager* mChunkManager;
	
	QQueue<LightNode> torchLightQueue;
	QQueue<LightRemovalNode> torchLightRemovalQueue;

	QQueue<LightNode> sunLightQueue;
	QQueue<LightRemovalNode> sunLightRemovalQueue;

	QHash<Coords, QQueue<LightNode>> torchLightUnloadedQueues;
	QHash<Coords, QQueue<LightRemovalNode>> torchLightRemovalUnloadedQueues;

	QHash<Coords, QQueue<LightNode>> sunLightUnloadedQueues;
	QHash<Coords, QQueue<LightRemovalNode>> sunLightRemovalUnloadedQueues;

	QQueue<Chunk*> dirtyLightMaps;
};

