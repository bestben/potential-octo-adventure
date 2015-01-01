#pragma once
#include "chunk.h"
#include <QtCore/QQueue>

#define MAX_LIGHT_UPDATES_PER_FRAME 512
class GameWindow;
class ChunkManager;

struct LightNode
{
	Coords pos;
	Chunk* chunk;
};

class LightManager
{
public:
	LightManager(ChunkManager* cm);
	~LightManager();
	void processChunk(Chunk* chunk);
	void placeTorchLight(Coords voxelCoords, uint8 amount);
	void update(GameWindow* gl);
	void processNodeNeighbor(LightNode& node, Coords dir, uint8 light, Voxel* currentData);
private:
	ChunkManager* mChunkManager;
	
	QQueue<LightNode> torchLightQueue;
	QHash<Coords, QQueue<LightNode>> torchLightUnloadedQueues;

	QQueue<Chunk*> dirtyLightMaps;
};

