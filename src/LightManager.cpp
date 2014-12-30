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

void LightManager::processChunk(Coords id) {
	

}
void LightManager::placeTorchLight(Coords voxelCoords, uint8 amount) {

	std::cout << "Placing light at" << voxelCoords.i << "," << voxelCoords.j << "," << voxelCoords.k << std::endl;

	

	Coords chunkCoords = voxelGetChunk(voxelCoords);
	auto* chunk = &mChunkManager->getChunk(chunkCoords);

	// Ne devrait jamais arriver car getChunk le crée s'il n'existe pas
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
			continue;
		}

		Voxel* data = mChunkManager->getBufferAdress(node.chunk->chunkBufferIndex);

		if (data == nullptr) {
			continue;
		}
		int index = getIndexInChunkData(node.pos);
		uint8 light = data[index].torchLight;

		Coords newPos = {};
		Chunk* newChunk;

		// X-1
		newPos = node.pos;
		newPos.i -= 1;
		newChunk = node.chunk;
		if (newPos.i < 0) {
			newPos.i += CHUNK_SIZE;
			newChunk = &mChunkManager->getChunk({ newChunk->i - 1, newChunk->j, newChunk->k });
			data = mChunkManager->getBufferAdress(newChunk->chunkBufferIndex);
		}
		if (data != nullptr) {
			int newIndex = getIndexInChunkData(newPos);
			Voxel* newVoxel = &data[newIndex];
			if (newVoxel->torchLight + 2 <= light) {
				newVoxel->torchLight = light - 1;
				dirtyLightMaps.push_back(newChunk);
				newChunk->isDirty = true;
				if (!isOpaque(*newVoxel))
					torchLightQueue.enqueue({ newPos, newChunk });
			}
		}

		// X+1
		newPos = node.pos;
		newPos.i += 1;
		newChunk = node.chunk;
		if (newPos.i >= CHUNK_SIZE) {
			newPos.i -= CHUNK_SIZE;
			newChunk = &mChunkManager->getChunk({ newChunk->i + 1, newChunk->j, newChunk->k });
			data = mChunkManager->getBufferAdress(newChunk->chunkBufferIndex);
		}
		if (data != nullptr) {
			int newIndex = getIndexInChunkData(newPos);
			Voxel* newVoxel = &data[newIndex];
			if (newVoxel->torchLight + 2 <= light) {
				newVoxel->torchLight = light - 1;
				dirtyLightMaps.push_back(newChunk);
				newChunk->isDirty = true;
				if (!isOpaque(*newVoxel))
					torchLightQueue.enqueue({ newPos, newChunk });
			}
		}

		// Y-1
		newPos = node.pos;
		newPos.j -= 1;
		newChunk = node.chunk;
		if (newPos.j < 0) {
			newPos.j += CHUNK_SIZE;
			if (newChunk->j == 0) {
				data = nullptr;
			}
			else {
				newChunk = &mChunkManager->getChunk({ newChunk->i, newChunk->j - 1, newChunk->k });
				data = mChunkManager->getBufferAdress(newChunk->chunkBufferIndex);
			}
		}
		if (data != nullptr) {
			int newIndex = getIndexInChunkData(newPos);
			Voxel* newVoxel = &data[newIndex];
			if (newVoxel->torchLight + 2 <= light) {
				newVoxel->torchLight = light - 1;
				dirtyLightMaps.push_back(newChunk);
				newChunk->isDirty = true;
				if (!isOpaque(*newVoxel))
					torchLightQueue.enqueue({ newPos, newChunk });
			}
		}

		// Y+1
		newPos = node.pos;
		newPos.j += 1;
		newChunk = node.chunk;
		if (newPos.j >= CHUNK_SIZE) {
			newPos.j -= CHUNK_SIZE;
			if (newChunk->j == 6) {
				data = nullptr;
			}
			else {
				newChunk = &mChunkManager->getChunk({ newChunk->i, newChunk->j + 1, newChunk->k });
				data = mChunkManager->getBufferAdress(newChunk->chunkBufferIndex);
			}
		}
		if (data != nullptr) {
			int newIndex = getIndexInChunkData(newPos);
			Voxel* newVoxel = &data[newIndex];
			if (newVoxel->torchLight + 2 <= light) {
				newVoxel->torchLight = light - 1;
				dirtyLightMaps.push_back(newChunk);
				newChunk->isDirty = true;
				if (!isOpaque(*newVoxel))
					torchLightQueue.enqueue({ newPos, newChunk });
			}
		}

		// Z-1
		newPos = node.pos;
		newPos.k -= 1;
		newChunk = node.chunk;
		if (newPos.k < 0) {
			newPos.k += CHUNK_SIZE;
			newChunk = &mChunkManager->getChunk({ newChunk->i, newChunk->j, newChunk->k - 1 });
			data = mChunkManager->getBufferAdress(newChunk->chunkBufferIndex);
		}
		if (data != nullptr) {
			int newIndex = getIndexInChunkData(newPos);
			Voxel* newVoxel = &data[newIndex];
			if (newVoxel->torchLight + 2 <= light) {
				newVoxel->torchLight = light - 1;
				dirtyLightMaps.push_back(newChunk);
				newChunk->isDirty = true;
				if (!isOpaque(*newVoxel))
					torchLightQueue.enqueue({ newPos, newChunk });
			}
		}

		// Z+1
		newPos = node.pos;
		newPos.k += 1;
		newChunk = node.chunk;
		if (newPos.k >= CHUNK_SIZE) {
			newPos.k += CHUNK_SIZE;
			newChunk = &mChunkManager->getChunk({ newChunk->i, newChunk->j, newChunk->k + 1 });
			data = mChunkManager->getBufferAdress(newChunk->chunkBufferIndex);
		}
		if (data != nullptr) {
			int newIndex = getIndexInChunkData(newPos);
			Voxel* newVoxel = &data[newIndex];
			if (newVoxel->torchLight + 2 <= light) {
				newVoxel->torchLight = light - 1;
				dirtyLightMaps.push_back(newChunk);
				newChunk->isDirty = true;
				if (!isOpaque(*newVoxel))
					torchLightQueue.enqueue({ newPos, newChunk });
			}
		}

	} // End loop

	while (!dirtyLightMaps.isEmpty()) {
		Chunk *chunk = dirtyLightMaps.front();
		dirtyLightMaps.pop_front();

		if (chunk->isDirty)
			mChunkManager->uploadLightMap(gl, chunk);

	}
}