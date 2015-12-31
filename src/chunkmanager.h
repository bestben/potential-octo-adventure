#pragma once

#include <glad/glad.h>
#include "utilities/time.h"

#include <atomic>
#include <thread>
#include <mutex>
#include <unordered_map>
#include <memory>

#include "chunk.h"
#include "meshgenerator.h"
#include "biomes/ChunkGenerator.h"
#include "LightManager.h"
#include "utility.h"

#include "ChunkIOManager.h"
#include "Memory/BuddyAllocator.h"

class OpenGLVertexArrayObject;
class OpenglProgramShader;
class OpenGLTexture;
class GameWindow;
class MeshGenerator;
class ChunkGenerator;

struct Buffer
{
	enum Slice
	{
		E_X_POS = 0,
		E_X_NEG = 1,
		E_Y_POS = 2,
		E_Y_NEG = 3,
		E_Z_POS = 4,
		E_Z_NEG = 5,
	};
	unsigned int		opaqueCount;
	unsigned int		waterCount;
	unsigned int		toUpOpaqueCount;
	unsigned int		toUpWaterCount;
	int					iBufferOffset; // Offset in the global buffer
	int					iAllocatedSize;
	int					iSlicesStart[ 6 ];
	int					iSlicesSize[ 6 ];
	bool				draw;
};
struct ChunkSortData;

/**
 * @brief Classe gérant les chunks devant être chargés/déchargés/affichés.
 */
class ChunkManager
{
public:
	ChunkManager( int worldSeed );
	~ChunkManager();

	void					initialize( GameWindow* gl );
	void					destroy( GameWindow* gl );
	void					update( GameWindow* gl );
	void					draw( GameWindow* gl );

	void					printVboMemory();

	Voxel*					getBufferAdress( int index );
	Chunk*					getChunk( Coords pos );
	Chunk*					getChunk( int i, int j, int k );

	MI_FORCE_INLINE Voxel	getVoxel( int x, int y, int z, bool* loaded = nullptr );
	MI_FORCE_INLINE Voxel	getVoxel( Coords c );

	LightManager&			getLightManager();
	VoxelType				placeVoxel( Coords pos, VoxelType type );
	void					removeVoxel( Coords pos );
	/**
	 * @brief Modifie un voxel et lance la reconstruction du mesh.
	 * @param x Coordonnée du voxel.
	 * @param y Coordonnée du voxel.
	 * @param z Coordonnée du voxel.
	 * @return L'ancien type du voxel.
	 */
	VoxelType				setVoxel( int x, int y, int z, VoxelType newType, uint8 light = NO_CHANGE );
	VoxelType				setVoxel( Coords c, VoxelType newType, uint8 light = NO_CHANGE );


protected:
	void					run();

private:
	int						getArrayIndex( Coords chunkPos, Coords center );
	int						getArrayIndex( int i, int j, int k, Coords center );
	Coords					getChunkCoords( int index, Coords center );

	std::thread								m_oThread;
	bool									m_bIsInit;

	BuddyAllocator							m_oBuddyAllocator;
	MeshGenerator*							m_pMeshGenerator;
	// Le shader affichant un chunk
	std::unique_ptr<OpenglProgramShader>	m_xProgram;
	// Le shader affichant l'eau
	std::unique_ptr<OpenglProgramShader>	m_xWaterProgram;
	// L'atlas de textures
	std::unique_ptr<OpenGLTexture>			m_xAtlas;

	OpenGLVertexArrayObject*				m_pVao;
	GLuint									m_iVbo;

	int										m_iPosAttr;
	int										m_iMatrixUniform;
	int										m_iChunkPosUniform;
	int										m_iLightMapUniform;

	int										m_iWaterMatrixUniform;
	int										m_iWaterChunkPosUniform;
	int										m_iWaterTimerUniform;

	// Le tableau contenant tous les voxels des chunks
	Voxel*									m_pChunkBuffers;

	Chunk*									m_pChunks;
	Chunk**									m_pChunksMapping[ 2 ];
	Coords									m_oChunksCenter[ 2 ];
	std::atomic<int>						m_iMapIndex;
	std::vector<Chunk*>						m_oNextFreeBuffer;

	// Le tableau des buffers opengl
	Buffer*									m_pOglBuffers;
	Chunk**									m_pChunkToDraw;
	ChunkSortData*							m_pChunkToSort;
	int										m_iChunkToDrawCount;

	Coords									m_oCurrentChunk;

	ChunkIOManager							m_oIOManager;
	/////////////////////////
	/// Variables du second thread
	/////////////////////////

	std::atomic<bool>						m_bNeedRegen;
	std::mutex								m_oMutexGenerateQueue;
	std::vector<Chunk*>						m_oToGenerateChunkData;
	GLuint*									m_pTempVertexData;
	ChunkGenerator*							m_pChunkGenerator;
	LightManager*							m_pLightManager;
	bool									m_bFirstUpdate;
	Time									m_oAnimationTime;
	std::atomic<Chunk*>						m_pLastChunk;
};
