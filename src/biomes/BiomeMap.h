#pragma once

#include <QtCore/QHash>
#include "BiomeLayer.h"
#include <memory>

class BiomeLayer;

class BiomeMap
{
public:
	BiomeMap();
	BiomeMap(int i, int j);
	BiomeLayer& getLayer(const QString& layerName);
	int getGroundLevel(int i, int k) const;
	Voxel getVoxelType(int i, int j, int k) const;
	void outputDebug() const;
	~BiomeMap();

private:
	QHash <QString, std::shared_ptr<BiomeLayer>> mLayers;
	double *mTunnels;
};

