#include "LightManager.h"
#include "chunkmanager.h"

#include <QtCore/QQueue>
#include <iostream>

LightManager::LightManager(ChunkManager* cm) : mChunkManager{ cm }
{
}


LightManager::~LightManager()
{
}

void LightManager::processChunk(Chunk* chunk) {
	Coords c = { chunk->i, chunk->j, chunk->k };
	auto it = torchLightUnloadedQueues.find(c);
	if (it == torchLightUnloadedQueues.end())
		return;

	for (LightNode node : *it) {
		// Update pointer just in case
		node.chunk = chunk;
		torchLightQueue.enqueue(node);
	}
	torchLightUnloadedQueues.remove({ chunk->i, chunk->j, chunk->k });
}
void LightManager::placeTorchLight(Coords voxelCoords, uint8 amount) {


	Coords chunkCoords = voxelGetChunk(voxelCoords);
	auto* chunk = mChunkManager->getChunk(chunkCoords);

	if (chunk == nullptr)
		return;
	
	// Si l'utilisateur essaye de placer une lumière sur un chunk pas chargé ya un problème !
	if (chunk->chunkBufferIndex == -1)
		return;

	Voxel* data = mChunkManager->getBufferAdress(chunk->chunkBufferIndex);

	auto pos = voxelCoordsToChunkCoords(voxelCoords);
	if (amount >= 16) amount = 15;

	data[getIndexInChunkData(pos)].torchLight = amount;
	torchLightQueue.enqueue({ pos, chunk });
	
	
}




void LightManager::update(GameWindow* gl) {
	int cnt = 0;
	dirtyLightMaps.clear();

	while (!torchLightQueue.isEmpty() && cnt < MAX_LIGHT_UPDATES_PER_FRAME) {
		cnt++;
		auto node = torchLightQueue.front();
		torchLightQueue.pop_front();

		if (node.chunk == nullptr) {
			// Ya un gros problème la !
			continue;
		}

		Voxel* data = mChunkManager->getBufferAdress(node.chunk->chunkBufferIndex);

		if (data == nullptr) {
			torchLightUnloadedQueues[{node.chunk->i, node.chunk->j, node.chunk->k}].enqueue(node);
			continue;
		}
		int index = getIndexInChunkData(node.pos);
		uint8 light = data[index].torchLight;

		processNodeNeighbor(node, { -1, 0, 0 }, light, data);
		processNodeNeighbor(node, { 1, 0, 0 }, light, data);
		processNodeNeighbor(node, { 0, -1, 0 }, light, data);
		processNodeNeighbor(node, { 0, 1, 0 }, light, data);
		processNodeNeighbor(node, { 0, 0, -1 }, light, data);
		processNodeNeighbor(node, { 0, 0, 1 }, light, data);


	} // End loop

	while (!dirtyLightMaps.isEmpty()) {
		Chunk *chunk = dirtyLightMaps.front();
		dirtyLightMaps.pop_front();

		if (chunk->isDirty)
			mChunkManager->uploadLightMap(gl, chunk);

	}
}

void LightManager::processNodeNeighbor(LightNode& node, Coords dir, uint8 light, Voxel* currentData) {
	
	Voxel* data = currentData;

	Coords newPos = node.pos + dir;
	Chunk* newChunk = node.chunk;

	if (newPos.i < 0) {
		newPos.i += CHUNK_SIZE;
		newChunk = mChunkManager->getChunk({ newChunk->i - 1, newChunk->j, newChunk->k });
		data = mChunkManager->getBufferAdress(newChunk->chunkBufferIndex);
	}
	else if (newPos.i >= CHUNK_SIZE) {
		newPos.i -= CHUNK_SIZE;
		newChunk = mChunkManager->getChunk({ newChunk->i + 1, newChunk->j, newChunk->k });
		data = mChunkManager->getBufferAdress(newChunk->chunkBufferIndex);
	}
	else if (newPos.j < 0) {
		newPos.j += CHUNK_SIZE;
		if (newChunk->j == 0) {
			return;
		}
		newChunk = mChunkManager->getChunk({ newChunk->i, newChunk->j - 1, newChunk->k });
		data = mChunkManager->getBufferAdress(newChunk->chunkBufferIndex);
	}
	else if (newPos.j >= CHUNK_SIZE) {
		newPos.j -= CHUNK_SIZE;
		if (newChunk->j == 6) {
			return;
		}
		newChunk = mChunkManager->getChunk({ newChunk->i, newChunk->j + 1, newChunk->k });
		data = mChunkManager->getBufferAdress(newChunk->chunkBufferIndex);
	}
	else if (newPos.k < 0) {
		newPos.k += CHUNK_SIZE;
		newChunk = mChunkManager->getChunk({ newChunk->i, newChunk->j, newChunk->k - 1 });
		data = mChunkManager->getBufferAdress(newChunk->chunkBufferIndex);
	}
	else if (newPos.k >= CHUNK_SIZE) {
		newPos.k -= CHUNK_SIZE;
		newChunk = mChunkManager->getChunk({ newChunk->i, newChunk->j, newChunk->k + 1 });
		data = mChunkManager->getBufferAdress(newChunk->chunkBufferIndex);
	}


	if (data != nullptr) {
		int newIndex = getIndexInChunkData(newPos);
		Voxel* newVoxel = &data[newIndex];
		if (!isOpaque(*newVoxel) && newVoxel->torchLight + 2 <= light) {
			newVoxel->torchLight = light - 1;
			dirtyLightMaps.push_back(newChunk);
			newChunk->isDirty = true;
			torchLightQueue.enqueue({ newPos, newChunk });
		}
	}
	else {
		torchLightUnloadedQueues[{node.chunk->i, node.chunk->j, node.chunk->k}].enqueue(node);
	}

}