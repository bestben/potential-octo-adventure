#include "BiomeMap.h"
#include "BiomeLayer.h"
#include <QtCore/QString>
#include "OpenSimplexNoise.hpp"

BiomeMap::BiomeMap() : BiomeMap(0,0){
	
}

BiomeMap::BiomeMap(int i, int k) : mLayers()
{
	mTunnels = new double[CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE * 7];

	BiomeLayer *rainfall = new BiomeLayer();

	rainfall->generate(255.0, 0.0, 0.05, (double)(i * CHUNK_SIZE), (double)(k * CHUNK_SIZE), 0);
	rainfall->outputDebugFile("rainfall.raw");
	mLayers.insert(QString("rainfall"), std::shared_ptr<BiomeLayer>(rainfall));


	// Temperature can varie betwenn -20°C and 30°C
	BiomeLayer *temperature = new BiomeLayer();
	temperature->generate(60.0, -20.0, 0.005, (double)(i * CHUNK_SIZE), (double)(k * CHUNK_SIZE), 1);
	temperature->outputDebugFile("temperature.raw");
	mLayers.insert(QString("temperature"), std::shared_ptr<BiomeLayer>(temperature));

	BiomeLayer *heightmap = new BiomeLayer();
	heightmap->generate(20.0, (double)GROUND_LEVEL, 0.05, (double)(i * CHUNK_SIZE), (double)(k * CHUNK_SIZE), 2);
	heightmap->outputDebugFile("heightmap.raw");
	mLayers.insert(QString("heightmap"), std::shared_ptr<BiomeLayer>(heightmap));

	BiomeLayer *mountains = new BiomeLayer();
	heightmap->generate(60.0, 0.0, 0.05, (double)(i * CHUNK_SIZE), (double)(k * CHUNK_SIZE), 2);
	heightmap->outputDebugFile("mountains.raw");
	mLayers.insert(QString("mountains"), std::shared_ptr<BiomeLayer>(mountains));

	OpenSimplexNoise tunnelNoise(-1);

	for (int x = 0; x < CHUNK_SIZE; ++x) {
		for (int y = 0; y < CHUNK_SIZE*7; ++y) {
			for (int z = 0; z < CHUNK_SIZE; ++z) {
				mTunnels[z*CHUNK_SIZE*CHUNK_SIZE + y*CHUNK_SIZE + x] = 50.0*(tunnelNoise.value(x + (i * CHUNK_SIZE), y, z + (k * CHUNK_SIZE))+1.0);
			}
		}
	}

}


BiomeMap::~BiomeMap()
{
	delete mTunnels;
}



BiomeLayer& BiomeMap::getLayer(const QString &layerName) {
	return *mLayers[layerName];
}

int BiomeMap::getGroundLevel(int i, int k) const {
	
	auto& height = *mLayers[QString("heightmap")];
	auto& mountains = *mLayers[QString("mountains")];

	return (int)(height.getValue(i, k) + mountains.getValue(i, k));
}

int BiomeMap::getVoxelType(int i, int j, int k) const{
	// TODO: Use other layers
	return (mTunnels[k*CHUNK_SIZE*CHUNK_SIZE + j*CHUNK_SIZE + i] > 50.0 && k<=getGroundLevel(i,k)) ? 1 : 0;
}
