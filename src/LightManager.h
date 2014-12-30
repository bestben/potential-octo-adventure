#pragma once
#include "chunk.h"
#include <QtCore/QQueue>

#define MAX_LIGHT_UPDATES_PER_FRAME 2048
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
	void processChunk(Coords id);
	void placeTorchLight(Coords voxelCoords, uint8 amount);
	void update(GameWindow* gl);
private:
	ChunkManager* mChunkManager;
	
	QQueue<LightNode> torchLightQueue;

	QQueue<Chunk*> dirtyLightMaps;
};

