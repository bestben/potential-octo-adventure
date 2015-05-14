#pragma once
#include "chunk.h"
#include <vector>
#include <set>
#include <unordered_map>

#define MAX_LIGHT_UPDATES_PER_FRAME 1024*32


class GameWindow;
class ChunkManager;


class LightManager
{
public:
	LightManager(ChunkManager* cm);
	~LightManager();
	

    void placeVoxel(Coords pos, VoxelType type, std::set<Coords> &modifiedChunks);
    void removeVoxel(Coords pos, std::set<Coords> &modifiedChunks);

	void updateLighting(Chunk* chunk);

    void lightNeighbors(Coords coords, std::set<Coords> &modifiedChunks);
    void unlightNeighbors(Coords coords, uint8 old_light, std::set<Coords> &light_sources, std::set<Coords> &modifiedChunks);
	
    bool propagateSunLight(Chunk* chunk, std::set<Coords>& lightSources, std::set<Coords> &modifiedChunks);
    uint16 propagateSunLight(Coords start, std::set<Coords> &modifiedChunks);

	
    void spreadLight(std::set<Coords>& lightSources, std::set<Coords> &modifiedChunks);
    void spreadLight(Coords* lightSources, int lightSourceCount, std::set<Coords> &modifiedChunks);
    void unspreadLight(std::unordered_map<Coords, uint8> &from, std::set<Coords> &lightSources, std::set<Coords> &modifiedChunks);

private:
	ChunkManager* mChunkManager;
	
};

