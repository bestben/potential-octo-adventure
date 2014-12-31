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
	if (it != torchLightUnloadedQueues.end()){
		for (LightNode node : *it) {
			node.chunk = chunk;
			torchLightQueue.enqueue(node);
		}
		torchLightUnloadedQueues.remove({ chunk->i, chunk->j, chunk->k });
	}

	auto itr = torchLightRemovalUnloadedQueues.find(c);
	if (itr != torchLightRemovalUnloadedQueues.end()){
		for (LightRemovalNode node : *itr) {
			node.chunk = chunk;
			torchLightRemovalQueue.enqueue(node);
		}
		torchLightRemovalUnloadedQueues.remove({ chunk->i, chunk->j, chunk->k });
	}

	auto its = sunLightUnloadedQueues.find(c);
	if (its != sunLightUnloadedQueues.end()){
		for (LightNode node : *its) {
			node.chunk = chunk;
			sunLightQueue.enqueue(node);
		}
		sunLightUnloadedQueues.remove({ chunk->i, chunk->j, chunk->k });
	}

	auto itsr = sunLightRemovalUnloadedQueues.find(c);
	if (itsr != sunLightRemovalUnloadedQueues.end()){
		for (LightRemovalNode node : *itsr) {
			node.chunk = chunk;
			sunLightRemovalQueue.enqueue(node);
		}
		sunLightRemovalUnloadedQueues.remove({ chunk->i, chunk->j, chunk->k });
	}
	
}

void LightManager::initializeSunlight(Chunk* chunk) {
	// Calcul de la lumière du soleil
	Voxel* data = nullptr;

	if (chunk->j < 6) {
		if (chunk->chunkYP) {
			// On assume que le sunlight est déjà calculé pour le chunk au dessus
			Chunk* top = chunk->chunkYP;
			data = mChunkManager->getBufferAdress(top->chunkBufferIndex);
			
			if (data != nullptr) {
				for (int i = 0; i < CHUNK_SIZE; i++) {
					for (int k = 0; k < CHUNK_SIZE; k++) {
						int index = getIndexInChunkData({ i, 0, k });
						if (data[index].sunLight != 0) {
							sunLightQueue.enqueue({ { i, 0, k }, top });
						}
					}
				}
			}
		}
		else {
			// Que faire ?
		}
	}
	else {
		// On est le chunk le plus haut, la couche la plus haut du monde génère la lumière du soleil
		data = mChunkManager->getBufferAdress(chunk->chunkBufferIndex);
		if (data != nullptr) {
			for (int i = 0; i < CHUNK_SIZE; i++) {
				for (int k = 0; k < CHUNK_SIZE; k++) {
					int index = getIndexInChunkData({ i, CHUNK_SIZE - 1, k });
					data[index].sunLight = 15;
					sunLightQueue.enqueue({ { i, CHUNK_SIZE - 1, k }, chunk });
				}
			}
		}
	}

	
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

void LightManager::voxelChanged(Coords voxelCoords) {


	Coords chunkCoords = voxelGetChunk(voxelCoords);
	auto* chunk = mChunkManager->getChunk(chunkCoords);

	if (chunk == nullptr)
		return;

	if (chunk->chunkBufferIndex == -1)
		return;

	Voxel* data = mChunkManager->getBufferAdress(chunk->chunkBufferIndex);

	auto pos = voxelCoordsToChunkCoords(voxelCoords);
	int index = getIndexInChunkData(pos);

	uint8 sun = data[index].sunLight;
	uint8 torch = data[index].torchLight;
	if (isOpaque(data[index])) {
		data[getIndexInChunkData(pos)].sunLight = 0;
		sunLightRemovalQueue.enqueue({ pos, chunk, sun });

		data[getIndexInChunkData(pos)].torchLight = 0;
		torchLightRemovalQueue.enqueue({ pos, chunk, torch });
	}
	else {
		//TODO: Repropager la lumière
	}
	


}


void LightManager::removeTorchLight(Coords voxelCoords) {


	Coords chunkCoords = voxelGetChunk(voxelCoords);
	auto* chunk = mChunkManager->getChunk(chunkCoords);

	if (chunk == nullptr)
		return;

	if (chunk->chunkBufferIndex == -1)
		return;

	Voxel* data = mChunkManager->getBufferAdress(chunk->chunkBufferIndex);

	auto pos = voxelCoordsToChunkCoords(voxelCoords);

	int index = getIndexInChunkData(pos);
	uint8 value = data[index].torchLight;
	torchLightRemovalQueue.enqueue({ pos, chunk, value});
	data[index].torchLight = 0;

}

void LightManager::update(GameWindow* gl) {
	int cnt = 0;
	dirtyLightMaps.clear();

	while (!torchLightRemovalQueue.isEmpty() && cnt < MAX_LIGHT_UPDATES_PER_FRAME) {
		cnt++;
		auto node = torchLightRemovalQueue.front();
		torchLightRemovalQueue.pop_front();

		if (node.chunk == nullptr) {
			//TODO: DO something maybe ?
			continue;
		}

		Voxel* data = mChunkManager->getBufferAdress(node.chunk->chunkBufferIndex);

		if (data == nullptr) {
			torchLightRemovalUnloadedQueues[{node.chunk->i, node.chunk->j, node.chunk->k}].enqueue(node);
			continue;
		}


		processNodeRemovalNeighbor(node, { -1, 0, 0 }, data);
		processNodeRemovalNeighbor(node, { 1, 0, 0 }, data);
		processNodeRemovalNeighbor(node, { 0, -1, 0 }, data);
		processNodeRemovalNeighbor(node, { 0, 1, 0 }, data);
		processNodeRemovalNeighbor(node, { 0, 0, -1 }, data);
		processNodeRemovalNeighbor(node, { 0, 0, 1 }, data);

	}

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


	while (!sunLightRemovalQueue.isEmpty() && cnt < MAX_LIGHT_UPDATES_PER_FRAME) {
		cnt++;
		auto node = sunLightRemovalQueue.front();
		sunLightRemovalQueue.pop_front();

		if (node.chunk == nullptr) {
			//TODO: DO something maybe ?
			continue;
		}

		Voxel* data = mChunkManager->getBufferAdress(node.chunk->chunkBufferIndex);

		if (data == nullptr) {
			sunLightRemovalUnloadedQueues[{node.chunk->i, node.chunk->j, node.chunk->k}].enqueue(node);
			continue;
		}

		processSunNodeRemovalNeighbor(node, { -1, 0, 0 }, data);
		processSunNodeRemovalNeighbor(node, { 1, 0, 0 }, data);
		processSunNodeRemovalNeighbor(node, { 0, -1, 0 }, data);
		processSunNodeRemovalNeighbor(node, { 0, 1, 0 }, data);
		processSunNodeRemovalNeighbor(node, { 0, 0, -1 }, data);
		processSunNodeRemovalNeighbor(node, { 0, 0, 1 }, data);

	}


	while (!sunLightQueue.isEmpty() && cnt < MAX_LIGHT_UPDATES_PER_FRAME) {
		cnt++;
		auto node = sunLightQueue.front();
		sunLightQueue.pop_front();

		if (node.chunk == nullptr) {
			// Ya un gros problème la !
			continue;
		}

		Voxel* data = mChunkManager->getBufferAdress(node.chunk->chunkBufferIndex);

		if (data == nullptr) {
			sunLightUnloadedQueues[{node.chunk->i, node.chunk->j, node.chunk->k}].enqueue(node);
			continue;
		}
		int index = getIndexInChunkData(node.pos);
		uint8 light = data[index].sunLight;

		processSunNodeNeighbor(node, { -1, 0, 0 }, light, data);
		processSunNodeNeighbor(node, { 1, 0, 0 }, light, data);
		processSunNodeNeighbor(node, { 0, -1, 0 }, light, data);
		processSunNodeNeighbor(node, { 0, 1, 0 }, light, data);
		processSunNodeNeighbor(node, { 0, 0, -1 }, light, data);
		processSunNodeNeighbor(node, { 0, 0, 1 }, light, data);


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

void LightManager::processNodeRemovalNeighbor(LightRemovalNode& node, Coords dir, Voxel* currentData) {

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
		uint8 neighborLevel = newVoxel->torchLight;

		if (neighborLevel != 0 && neighborLevel < node.value) {
			newVoxel->torchLight = 0;
			dirtyLightMaps.push_back(newChunk);
			newChunk->isDirty = true;
			torchLightRemovalQueue.enqueue({ newPos, newChunk, neighborLevel });
		}
		else if (neighborLevel >= node.value) {
			torchLightQueue.enqueue({newPos, newChunk});
		}
	}
	else {
		torchLightRemovalUnloadedQueues[{node.chunk->i, node.chunk->j, node.chunk->k}].enqueue(node);
	}

}

void LightManager::processSunNodeNeighbor(LightNode& node, Coords dir, uint8 light, Voxel* currentData) {

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
		if (!isOpaque(*newVoxel) && newVoxel->sunLight + 2 <= light) {
			if (dir != Coords{0,-1,0})
				newVoxel->sunLight = light - 1;
			else
				newVoxel->sunLight = light;

			dirtyLightMaps.push_back(newChunk);
			newChunk->isDirty = true;
			sunLightQueue.enqueue({ newPos, newChunk });
		}
	}
	else {
		sunLightUnloadedQueues[{node.chunk->i, node.chunk->j, node.chunk->k}].enqueue(node);
	}

}

void LightManager::processSunNodeRemovalNeighbor(LightRemovalNode& node, Coords dir, Voxel* currentData) {

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
		uint8 neighborLevel = newVoxel->sunLight;

		if ((node.value == 15 && dir == Coords{ 0, -1, 0 } )|| (neighborLevel != 0 && neighborLevel < node.value)) {
			newVoxel->sunLight = 0;
			dirtyLightMaps.push_back(newChunk);
			newChunk->isDirty = true;
			sunLightRemovalQueue.enqueue({ newPos, newChunk, neighborLevel });
		}
		else if (neighborLevel >= node.value) {
			sunLightQueue.enqueue({ newPos, newChunk });
		}
	}
	else {
		sunLightRemovalUnloadedQueues[{node.chunk->i, node.chunk->j, node.chunk->k}].enqueue(node);
	}

}
