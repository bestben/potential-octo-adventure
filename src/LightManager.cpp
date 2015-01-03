#include "LightManager.h"
#include "chunkmanager.h"

#include <QtCore/QQueue>
#include <QtCore/QSet>
#include <iostream>
#include <QtCore/QTimer>

LightManager::LightManager(ChunkManager* cm) : mChunkManager{ cm }
{
}

LightManager::~LightManager()
{
}



void LightManager::placeVoxel(Coords pos, VoxelType type, QSet<Coords> &modifiedChunks){

	Voxel n = {};
	n.type = type;
	
	QSet<Coords> lightSources;

	Voxel v = mChunkManager->getVoxel(pos);

	if (v.type == VoxelType::IGNORE_TYPE)
		return;

	Voxel above = mChunkManager->getVoxel(pos + Coords{ 0, 1, 0 });
	bool underSun = (above.type != VoxelType::IGNORE_TYPE && above.getLight() == SUN_LIGHT);

	

	uint8 oldlight = v.getLight();
	

	unlightNeighbors(pos, oldlight, lightSources, modifiedChunks);
	n._light = 0;


	if (!isOpaque(v) && underSun) {
		n._light = SUN_LIGHT;
	}

	mChunkManager->setVoxel(pos, n.type, n._light);
	modifiedChunks.insert(GetChunkPosFromVoxelPos(pos));

	if (isOpaque(v) && underSun) {

		uint16 y = pos.j - 1;

		for (;; --y) {
			Coords newPos = { pos.i, y, pos.k };

			Voxel newVoxel = mChunkManager->getVoxel(newPos);

			if (newVoxel.type == VoxelType::IGNORE_TYPE)
				break;

			if (newVoxel.getLight() == SUN_LIGHT) {
				//TODO: Verifier les variables passées en parametre
				unlightNeighbors(newPos, newVoxel.getLight(), lightSources, modifiedChunks);
				mChunkManager->setVoxel(newPos, newVoxel.type, 0);
				modifiedChunks.insert(GetChunkPosFromVoxelPos(newPos));
			}
			else
				break;
			

		}

	}

	const Coords neighbors[6] = {
		Coords{ 0, 0, 1 },
		Coords{ 0, 1, 0 },
		Coords{ 1, 0, 0 },
		Coords{ 0, 0, -1 },
		Coords{ 0, -1, 0 },
		Coords{ -1, 0, 0 }
	};


	for (uint16 dir = 0; dir < 6; ++dir) {
		Coords p2 = pos + neighbors[dir];
		lightSources.insert(p2);
		continue;
	}

	lightSources.insert(pos);
	spreadLight(lightSources, modifiedChunks);

}

void LightManager::removeVoxel(Coords pos, QSet<Coords> &modifiedChunks) {
	
	Voxel n = {};
	n.type = VoxelType::AIR;

	QSet<Coords> lightSources;

	Voxel v = mChunkManager->getVoxel(pos);

	if (v.type == VoxelType::IGNORE_TYPE)
		return;

	Voxel above = mChunkManager->getVoxel(pos + Coords{ 0, 1, 0 });
	bool underSun = (above.type != VoxelType::IGNORE_TYPE && above.getLight() == SUN_LIGHT);

	
	unlightNeighbors(pos, v.getLight(), lightSources, modifiedChunks);
	
	mChunkManager->setVoxel(pos, n.type, 0);


	spreadLight(lightSources, modifiedChunks);

	modifiedChunks.insert(GetChunkPosFromVoxelPos(pos));

	if (underSun) {

		uint16 bottom = propagateSunLight(pos, modifiedChunks);

		uint16 y = pos.j;
		for (; y >= bottom; --y) {
			Coords newPos = { pos.i, y, pos.k };

			Voxel newVoxel = mChunkManager->getVoxel(newPos);

			if (newVoxel.type == VoxelType::IGNORE_TYPE)
				break;

			lightNeighbors(newPos, modifiedChunks);

		}

	}
	else {
		// Inutile
		// mChunkManager->setVoxel(pos, n.type, 0);
	}
	
	const Coords neighbors[6] = {
		Coords{ 0, 0, 1 },
		Coords{ 0, 1, 0 },
		Coords{ 1, 0, 0 },
		Coords{ 0, 0, -1 },
		Coords{ 0, -1, 0 },
		Coords{ -1, 0, 0 }
	};

	uint8 maxLight = 0;
	Coords maxLightPos = {};
	bool found = false;


	for (uint16 dir = 0; dir < 6; ++dir) {
		Coords p2 = pos + neighbors[dir];
		Voxel v2 = mChunkManager->getVoxel(p2);
		if (v2.type == VoxelType::IGNORE_TYPE)
			continue;

		if (v2.getLight() > maxLight || !found) {
			maxLight = v2.getLight();
			maxLightPos = p2;
			found = true;
		}


	}

	if (found) {
		lightNeighbors(maxLightPos, modifiedChunks);
	}
	

}

void LightManager::updateLighting(Chunk* chunk) {
	
	QTime timer;
	timer.start();

	QHash<Coords, uint8> removelightFrom;

	QSet<Coords> lightSources;
	QSet<Coords> modifiedChunks;

	Chunk* current_chunk = chunk;
	while (true) {

		if (current_chunk->chunkBufferIndex == -1)
			break;

		Coords chunkPos = { chunk->i, chunk->j, chunk->k };
		Coords offset = chunkPos * CHUNK_SIZE;

		modifiedChunks.insert(chunkPos);

		for (int k = 0; k < CHUNK_SIZE; k++)
		for (int j = 0; j < CHUNK_SIZE; j++)
		for (int i = 0; i < CHUNK_SIZE; i++) {
			Coords pos = { i, j, k };
			Coords worldPos = offset + pos;
			auto voxel = mChunkManager->getVoxel(worldPos);

			if (voxel.type == VoxelType::IGNORE_TYPE)
				continue;

			uint8 oldlight = voxel.getLight();

			mChunkManager->setVoxel(worldPos, voxel.type, 0);

			if (lightSource(voxel.type) > 0)
				lightSources.insert(worldPos);
			

			if (atChunkBounds(pos)) {
				if (oldlight != 0) {
					removelightFrom[worldPos] = oldlight;
				} else {
					lightSources.insert(worldPos);
				}
			}

		}

		bool shouldGoDown = propagateSunLight(current_chunk, lightSources, modifiedChunks);


		if (!shouldGoDown)
			break;
		
		current_chunk = mChunkManager->getChunk(chunkPos + Coords{0, -1, 0});
		

	}
	
	unspreadLight(removelightFrom, lightSources, modifiedChunks);
	
	spreadLight(lightSources, modifiedChunks);

	for (auto coords : modifiedChunks) {
		Chunk* chunkModified = mChunkManager->getChunk(coords);
		if (chunkModified != nullptr) {
			chunkModified->isLightDirty = true;
		}
	}


}

bool LightManager::propagateSunLight(Chunk* chunk, QSet<Coords> &lightSources, QSet<Coords> &modifiedChunks) {
	bool shouldGoDown = false;

	Coords chunkPos = { chunk->i, chunk->j, chunk->k };
	Coords offset = chunkPos * CHUNK_SIZE;

	for (int k = 0; k < CHUNK_SIZE; ++k)
	for (int i = 0; i < CHUNK_SIZE; ++i) {
		
		bool sunAbove = true;
		Voxel above = mChunkManager->getVoxel(offset + Coords{ i, CHUNK_SIZE, k });

		if (above.type == VoxelType::IGNORE_TYPE) {
			// On assume qu'il y a du soleil au dessus
			// TODO: Définir à la génération si le chunk est considéré sous-terre ou non et l'utiliser ici
			sunAbove = (chunk->j+1)*CHUNK_SIZE >= GROUND_LEVEL;
		} else {
			if (above.getLight() == SUN_LIGHT)
				sunAbove = true;
			else
				sunAbove = false;
		}

		uint8 currentLight = sunAbove ? SUN_LIGHT : 0;

		for (int j = CHUNK_SIZE - 1; j >= 0; --j){
			Coords pos = { i, j, k };

			Voxel v = mChunkManager->getVoxel(offset + pos);

			if (v.type == VoxelType::IGNORE_TYPE)
				continue;

			if (currentLight == 0) {
				// Rien
			} else if (currentLight == SUN_LIGHT && !isOpaque(v)) {
				// La lumière continue sans diminuer
			} else if (isOpaque(v)) {
				currentLight = 0;
			} else {
				currentLight = reduce_light(currentLight);
			}

			uint8 oldlight = v.getLight();
			if (currentLight > oldlight) {
				mChunkManager->setVoxel(offset + pos, v.type, currentLight);
				modifiedChunks.insert(GetChunkPosFromVoxelPos(offset + pos));
			}

			if (reduce_light(currentLight) != 0) {
				lightSources.insert(offset + pos);
			}

		}

		bool needMorePropagation = currentLight == SUN_LIGHT;

		if (!shouldGoDown && chunkPos.j>0) {
			Coords pos = { i, -1, k };
			Voxel below = mChunkManager->getVoxel(offset + pos);

			if (!isOpaque(below)) {
				if (!needMorePropagation && below.getLight() == SUN_LIGHT)
					shouldGoDown = true;
				else if (needMorePropagation && below.getLight() != SUN_LIGHT)
					shouldGoDown = true;
			}
		}
	}
	
	return shouldGoDown;
}

uint16 LightManager::propagateSunLight(Coords start, QSet<Coords> &modifiedChunks) {
	
	uint16 y = start.j;

	for (;; --y) {
		Coords pos = { start.i, y, start.k };

		Voxel v = mChunkManager->getVoxel(pos);

		if (v.type == VoxelType::IGNORE_TYPE)
			break;

		if (!isOpaque(v)) {
			mChunkManager->setVoxel(pos, v.type, SUN_LIGHT);
			modifiedChunks.insert(GetChunkPosFromVoxelPos(pos));
		}

	}

	return y + 1;
}

void LightManager::unspreadLight(QHash<Coords, uint8> &from, QSet<Coords> &lightSources, QSet<Coords> &modifiedChunks) {
	
	const Coords neighbors[6] = {
			Coords{  0,  0,  1 },
			Coords{  0,  1,  0 },
			Coords{  1,  0,  0 },
			Coords{  0,  0, -1 },
			Coords{  0, -1,  0 },
			Coords{ -1,  0,  0 }
	};

	if (from.isEmpty())
		return;


	QHash<Coords, uint8> unlightedSun;



	for (auto it = from.begin(); it != from.end(); ++it) {

		Coords pos = it.key();
		uint8 oldlight = *it;

		for (uint16 dir = 0; dir < 6; ++dir) {
			Coords newPos = pos + neighbors[dir];
			Voxel v = mChunkManager->getVoxel(newPos);

			if (v.type == VoxelType::IGNORE_TYPE)
				continue;

			if (v.getLight() < oldlight && v.getLight() != 0) {
				if (!isOpaque(v)) {
					unlightedSun[newPos] = v.getLight();
					mChunkManager->setVoxel(newPos, v.type, 0);
					modifiedChunks.insert(GetChunkPosFromVoxelPos(newPos));
				}
			}
			else if (v.getLight() >= oldlight){
				lightSources.insert(newPos);
			}

		}

	}

	

	if (!unlightedSun.isEmpty()) {
		unspreadLight(unlightedSun, lightSources, modifiedChunks);
	}

}

void LightManager::unlightNeighbors(Coords coords, uint8 old_light, QSet<Coords> light_sources, QSet<Coords> &modifiedChunks) {

	QHash<Coords, uint8> fromSun;
	fromSun[coords] = old_light;
	unspreadLight(fromSun, light_sources, modifiedChunks);

}

void LightManager::spreadLight(QSet<Coords> &lightSources, QSet<Coords> &modifiedChunks) {

	if (lightSources.isEmpty())
		return;

	const Coords neighbors[6] = {
		Coords{ 0, 0, 1 },
		Coords{ 0, 1, 0 },
		Coords{ 1, 0, 0 },
		Coords{ 0, 0, -1 },
		Coords{ 0, -1, 0 },
		Coords{ -1, 0, 0 }
	};

	QSet<Coords> lighted;

	for (auto it = lightSources.begin(); it != lightSources.end(); ++it) {
		
		Coords pos = *it;

		Voxel v = mChunkManager->getVoxel(pos);

		if (v.type == VoxelType::IGNORE_TYPE)
			continue;

		uint8 oldlight = v.getLight();

		uint8 newlight = reduce_light(oldlight);

		

		for (uint16 dir = 0; dir < 6; dir++) {
			
			Coords newPos = pos + neighbors[dir];

			Voxel newVoxel = mChunkManager->getVoxel(newPos);

			if (newVoxel.type == VoxelType::IGNORE_TYPE)
				continue;

			if (newVoxel.getLight() > unreduce_light(oldlight)) {
				lighted.insert(newPos);
				modifiedChunks.insert(GetChunkPosFromVoxelPos(newPos));
			}
			if (newVoxel.getLight() < newlight && !isOpaque(newVoxel)) {
				mChunkManager->setVoxel(newPos, newVoxel.type, newlight);
				modifiedChunks.insert(GetChunkPosFromVoxelPos(newPos));
				lighted.insert(newPos);
			}


		}

	}

	if (!lighted.isEmpty())
		spreadLight(lighted, modifiedChunks);

}

void LightManager::lightNeighbors(Coords coords, QSet<Coords> &modifiedChunks) {
	QSet<Coords> from;
	from.insert(coords);
	spreadLight(from, modifiedChunks);
}






#if 0

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

	auto its = lightUnloadedQueues.find(c);
	if (its != lightUnloadedQueues.end()){
		for (LightNode node : *its) {
			node.chunk = chunk;
			lightQueue.enqueue(node);
		}
		lightUnloadedQueues.remove({ chunk->i, chunk->j, chunk->k });
	}

	auto itsr = lightRemovalUnloadedQueues.find(c);
	if (itsr != lightRemovalUnloadedQueues.end()){
		for (LightRemovalNode node : *itsr) {
			node.chunk = chunk;
			lightRemovalQueue.enqueue(node);
		}
		lightRemovalUnloadedQueues.remove({ chunk->i, chunk->j, chunk->k });
	}
	
}

void LightManager::initializelight(Chunk* chunk) {
	// Calcul de la lumière du soleil
	Voxel* data = nullptr;

	if (chunk->j < 6) {
		if (chunk->chunkYP) {
			// On assume que le light est déjà calculé pour le chunk au dessus
			Chunk* top = chunk->chunkYP;
			data = mChunkManager->getBufferAdress(top->chunkBufferIndex);
			
			if (data != nullptr) {
				for (int i = 0; i < CHUNK_SIZE; i++) {
					for (int k = 0; k < CHUNK_SIZE; k++) {
						int index = IndexVoxelRelPos({ i, 0, k });
						if (data[index].light != 0) {
							lightQueue.enqueue({ { i, 0, k }, top });
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
					int index = IndexVoxelRelPos({ i, CHUNK_SIZE - 1, k });
					data[index].light = 15;
					lightQueue.enqueue({ { i, CHUNK_SIZE - 1, k }, chunk });
				}
			}
		}
	}

	
}

void LightManager::placeTorchLight(Coords voxelCoords, uint8 amount) {


	Coords chunkCoords = GetChunkPosFromVoxelPos(voxelCoords);
	auto* chunk = mChunkManager->getChunk(chunkCoords);

	if (chunk == nullptr)
		return;
	
	// Si l'utilisateur essaye de placer une lumière sur un chunk pas chargé ya un problème !
	if (chunk->chunkBufferIndex == -1)
		return;

	Voxel* data = mChunkManager->getBufferAdress(chunk->chunkBufferIndex);

	auto pos = GetVoxelRelPos(voxelCoords);
	if (amount >= 16) amount = 15;

	data[IndexVoxelRelPos(pos)].torchLight = amount;
	torchLightQueue.enqueue({ pos, chunk });
	
	
}

void LightManager::voxelChanged(Coords voxelCoords) {


	Coords chunkCoords = GetChunkPosFromVoxelPos(voxelCoords);
	auto* chunk = mChunkManager->getChunk(chunkCoords);

	if (chunk == nullptr)
		return;

	if (chunk->chunkBufferIndex == -1)
		return;

	Voxel* data = mChunkManager->getBufferAdress(chunk->chunkBufferIndex);

	auto pos = GetVoxelRelPos(voxelCoords);
	int index = IndexVoxelRelPos(pos);

	uint8 sun = data[index].light;
	uint8 torch = data[index].torchLight;
	if (isOpaque(data[index])) {
		data[IndexVoxelRelPos(pos)].light = 0;
		lightRemovalQueue.enqueue({ pos, chunk, sun });

		data[IndexVoxelRelPos(pos)].torchLight = 0;
		torchLightRemovalQueue.enqueue({ pos, chunk, torch });
	}
	else {
		//TODO: Repropager la lumière
	}
	


}

void LightManager::removeTorchLight(Coords voxelCoords) {


	Coords chunkCoords = GetChunkPosFromVoxelPos(voxelCoords);
	auto* chunk = mChunkManager->getChunk(chunkCoords);

	if (chunk == nullptr)
		return;

	if (chunk->chunkBufferIndex == -1)
		return;

	Voxel* data = mChunkManager->getBufferAdress(chunk->chunkBufferIndex);

	auto pos = GetVoxelRelPos(voxelCoords);

	int index = IndexVoxelRelPos(pos);
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
		int index = IndexVoxelRelPos(node.pos);
		uint8 light = data[index].torchLight;

		processNodeNeighbor(node, { -1, 0, 0 }, light, data);
		processNodeNeighbor(node, { 1, 0, 0 }, light, data);
		processNodeNeighbor(node, { 0, -1, 0 }, light, data);
		processNodeNeighbor(node, { 0, 1, 0 }, light, data);
		processNodeNeighbor(node, { 0, 0, -1 }, light, data);
		processNodeNeighbor(node, { 0, 0, 1 }, light, data);


	} // End loop


	while (!lightRemovalQueue.isEmpty() && cnt < MAX_LIGHT_UPDATES_PER_FRAME) {
		cnt++;
		auto node = lightRemovalQueue.front();
		lightRemovalQueue.pop_front();

		if (node.chunk == nullptr) {
			//TODO: DO something maybe ?
			continue;
		}

		Voxel* data = mChunkManager->getBufferAdress(node.chunk->chunkBufferIndex);

		if (data == nullptr) {
			lightRemovalUnloadedQueues[{node.chunk->i, node.chunk->j, node.chunk->k}].enqueue(node);
			continue;
		}

		processSunNodeRemovalNeighbor(node, { -1, 0, 0 }, data);
		processSunNodeRemovalNeighbor(node, { 1, 0, 0 }, data);
		processSunNodeRemovalNeighbor(node, { 0, -1, 0 }, data);
		processSunNodeRemovalNeighbor(node, { 0, 1, 0 }, data);
		processSunNodeRemovalNeighbor(node, { 0, 0, -1 }, data);
		processSunNodeRemovalNeighbor(node, { 0, 0, 1 }, data);

	}


	while (!lightQueue.isEmpty() && cnt < MAX_LIGHT_UPDATES_PER_FRAME) {
		cnt++;
		auto node = lightQueue.front();
		lightQueue.pop_front();

		if (node.chunk == nullptr) {
			// Ya un gros problème la !
			continue;
		}

		Voxel* data = mChunkManager->getBufferAdress(node.chunk->chunkBufferIndex);

		if (data == nullptr) {
			lightUnloadedQueues[{node.chunk->i, node.chunk->j, node.chunk->k}].enqueue(node);
			continue;
		}
		int index = IndexVoxelRelPos(node.pos);
		uint8 light = data[index].light;

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
		int newIndex = IndexVoxelRelPos(newPos);
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
		int newIndex = IndexVoxelRelPos(newPos);
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
		int newIndex = IndexVoxelRelPos(newPos);
		Voxel* newVoxel = &data[newIndex];
		if (!isOpaque(*newVoxel) && newVoxel->light + 2 <= light) {
			if (dir != Coords{0,-1,0})
				newVoxel->light = light - 1;
			else
				newVoxel->light = light;

			dirtyLightMaps.push_back(newChunk);
			newChunk->isDirty = true;
			lightQueue.enqueue({ newPos, newChunk });
		}
	}
	else {
		lightUnloadedQueues[{node.chunk->i, node.chunk->j, node.chunk->k}].enqueue(node);
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
		int newIndex = IndexVoxelRelPos(newPos);
		Voxel* newVoxel = &data[newIndex];
		uint8 neighborLevel = newVoxel->light;

		if ((node.value == 15 && dir == Coords{ 0, -1, 0 } )|| (neighborLevel != 0 && neighborLevel < node.value)) {
			newVoxel->light = 0;
			dirtyLightMaps.push_back(newChunk);
			newChunk->isDirty = true;
			lightRemovalQueue.enqueue({ newPos, newChunk, neighborLevel });
		}
		else if (neighborLevel >= node.value) {
			lightQueue.enqueue({ newPos, newChunk });
		}
	}
	else {
		lightRemovalUnloadedQueues[{node.chunk->i, node.chunk->j, node.chunk->k}].enqueue(node);
	}

}

#endif