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

			//lightSources.insert(newPos);
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
		Coords chunkPos = { current_chunk->i, current_chunk->j, current_chunk->k };
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

		if (current_chunk == nullptr)
			break;

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
			}

			if (reduce_light(currentLight) != 0) {
				lightSources.insert(offset + pos);
			}

			modifiedChunks.insert(GetChunkPosFromVoxelPos(offset + pos));

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
		}
		modifiedChunks.insert(GetChunkPosFromVoxelPos(pos));

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

	QHash<Coords, uint8> unlighted;

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
					unlighted[newPos] = v.getLight();
					mChunkManager->setVoxel(newPos, v.type, 0);
				}
			}
			else if (v.getLight() >= oldlight){
				lightSources.insert(newPos);
			}
			modifiedChunks.insert(GetChunkPosFromVoxelPos(newPos));
		}

	}

	

	if (!unlighted.isEmpty()) {
		unspreadLight(unlighted, lightSources, modifiedChunks);
	}

}

void LightManager::unlightNeighbors(Coords coords, uint8 old_light, QSet<Coords> &light_sources, QSet<Coords> &modifiedChunks) {

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
			}
			if (newVoxel.getLight() < newlight && !isOpaque(newVoxel)) {
				mChunkManager->setVoxel(newPos, newVoxel.type, newlight);
				lighted.insert(newPos);
			}
			modifiedChunks.insert(GetChunkPosFromVoxelPos(newPos));

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


