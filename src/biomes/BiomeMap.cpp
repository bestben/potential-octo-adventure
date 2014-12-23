#include "BiomeMap.h"
#include "BiomeLayer.h"

BiomeMap::BiomeMap() : mLayers()
{
	
	//mLayers.insert(std::string("rainfall"), new BiomeLayer());	

	
	BiomeLayer *rainfall = new BiomeLayer();

	rainfall->generate(255.0, 0.0, 0.05, 0.0, 0.0, 0);
	rainfall->outputDebugFile("rainfall.raw");

	BiomeLayer *temperature = new BiomeLayer();

	temperature->generate(255.0, 0.0, 0.005, 0.0, 0.0, 1);
	temperature->outputDebugFile("temperature.raw");



	delete rainfall;
	delete temperature;

}


BiomeMap::~BiomeMap()
{
}
