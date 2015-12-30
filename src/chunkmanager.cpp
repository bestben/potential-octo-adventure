#include "chunkmanager.h"
#include "gamewindow.h"

#include <iostream>
#include <algorithm>

#include "utilities/openglvertexarrayobject.h"
#include "utilities/openglprogramshader.h"
#include "utilities/opengltexture.h"

#define GLM_FORCE_PURE
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#define GRAPHIC_BUFFER_SIZE 1024 * 1024 * 128
#define MAX_REMESH_PER_UPDATE 4

struct ChunkSortData
{
	int		iChunkIndex;
	int		iDistance;
};

MI_FORCE_INLINE bool chunkSortFunction( const ChunkSortData& i, const ChunkSortData& j )
{
	return i.iDistance < j.iDistance;
}

ChunkManager::ChunkManager( int worldSeed )
	: m_bIsInit { false }
	, m_oBuddyAllocator { GRAPHIC_BUFFER_SIZE }
	, m_pChunkBuffers { nullptr }
	, m_pOglBuffers { nullptr }
	, m_bFirstUpdate { true }
	, m_oIOManager { worldSeed }
{
	m_iWorldSeed = worldSeed;
	m_pLightManager = new LightManager( this );
	m_pMeshGenerator = new MeshGenerator( this );
	m_pChunkGenerator = new ChunkGenerator( this, worldSeed );
	m_pChunkBuffers = new Voxel[ CHUNK_NUMBER * CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE ];
	memset( m_pChunkBuffers, 0, CHUNK_NUMBER * CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE * sizeof( Voxel ) );

	m_iChunkToDrawCount = 0;
	m_pChunkToDraw = new Chunk*[ VBO_NUMBER ];
	for( int i = 0; i < VBO_NUMBER; ++i )
	{
		m_pChunkToDraw[ i ] = nullptr;
	}
	m_pChunkToSort = new ChunkSortData[ VBO_NUMBER ];

	m_bNeedRegen = true;

	m_pTempVertexData = new GLuint[ CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE * 6 * 6 ];

	m_oCurrentChunk = { 0, -1, 0 };
	m_oAnimationTime.start();

	m_pChunks = new Chunk[ CHUNK_NUMBER ];
	m_pChunksMapping[ 0 ] = new Chunk*[ CHUNK_NUMBER ];
	m_pChunksMapping[ 1 ] = new Chunk*[ CHUNK_NUMBER ];

	int layerSize = ( FULL_VIEW_SIZE * FULL_VIEW_SIZE );
	for( int n = 0; n < CHUNK_NUMBER; ++n )
	{
		int j = div_floor( n, layerSize );

		int indexInLayer = n % layerSize;
		int i = ( indexInLayer % ( 2 * VIEW_SIZE + 1 ) ) - VIEW_SIZE;
		int k = ( indexInLayer / ( 2 * VIEW_SIZE + 1 ) ) - VIEW_SIZE;
		m_pChunks[ n ].i = i;
		m_pChunks[ n ].j = j;
		m_pChunks[ n ].k = k;

		m_pChunks[ n ].data = m_pChunkBuffers + n * CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE;
		m_pChunks[ n ].vboIndex = n;

		m_pChunksMapping[ 0 ][ n ] = m_pChunks + n;
		m_pChunksMapping[ 1 ][ n ] = m_pChunks + n;

		m_oToGenerateChunkData.push_back( m_pChunks + n );
		m_pChunks[ n ].inQueue = true;
		m_pChunks[ n ].isDirty = true;
		m_pChunks[ n ].isLightDirty = true;
	}
	m_oChunksCenter[ 0 ] = { 0, 0, 0 };
	m_oChunksCenter[ 1 ] = { 0, 0, 0 };
	m_iMapIndex = 0;
	m_pLastChunk = nullptr;
}

ChunkManager::~ChunkManager()
{

	for( int i = 0; i < CHUNK_NUMBER; ++i )
	{
		Chunk* chunk = m_pChunks + i;
		if( chunk->generated && chunk->differsFromDisk )
		{
			Voxel* voxel_data = chunk->data;
			m_oIOManager.saveChunk( Coords { chunk->i, chunk->j, chunk->k }, voxel_data );
		}
	}

	delete[] m_pChunkBuffers;
	delete[] m_pTempVertexData;
	delete[] m_pChunkToDraw;
	delete[] m_pChunkToSort;

	delete m_pChunkGenerator;
	delete m_pLightManager;
	delete m_pMeshGenerator;

	delete[] m_pChunks;
	delete[]m_pChunksMapping[ 0 ];
	delete[]m_pChunksMapping[ 1 ];
}

void ChunkManager::initialize( GameWindow* /*gl*/ )
{
	m_xProgram = std::make_unique<OpenglProgramShader>();
	m_xProgram->addShaderFromSourceFile( OpenGLShaderType::Vertex, "shaders/render.vs" );
	m_xProgram->addShaderFromSourceFile( OpenGLShaderType::Fragment, "shaders/render.ps" );
	if( !m_xProgram->link() )
	{
		// TODO(antoine): Remove Force crash
		MI_ASSERT( false );
		abort();
	}
	m_iPosAttr = m_xProgram->attributeLocation( "position" );
	m_iMatrixUniform = m_xProgram->uniformLocation( "viewProj" );
	m_iChunkPosUniform = m_xProgram->uniformLocation( "chunkPosition" );

	glm::vec3 skyColor( 0.53f, 0.807f, 0.92f );

	m_xProgram->bind();
	m_xProgram->setUniformValue( "atlas", 0 );
	m_xProgram->setUniformValue( "tileCount", 16 );
	m_xProgram->setUniformValue( "tileSize", 16 );
	m_xProgram->setUniformValue( "fogDistance", ( float ) ( VIEW_SIZE*CHUNK_SIZE*CHUNK_SCALE )*3.0f );

	glUniform3fv( m_xProgram->uniformLocation( "fogColor" ), 1, glm::value_ptr( skyColor ) );
	//m_xProgram->setUniformValue("fogColor", skyColor);
	m_xProgram->release();

	m_xWaterProgram = std::make_unique<OpenglProgramShader>();
	m_xWaterProgram->addShaderFromSourceFile( OpenGLShaderType::Vertex, "shaders/water.vs" );
	m_xWaterProgram->addShaderFromSourceFile( OpenGLShaderType::Fragment, "shaders/water.ps" );
	if( !m_xWaterProgram->link() )
	{
		// TODO(antoine): Remove Force crash
		MI_ASSERT( false );
		abort();
	}
	m_iWaterMatrixUniform = m_xWaterProgram->uniformLocation( "viewProj" );
	m_iWaterChunkPosUniform = m_xWaterProgram->uniformLocation( "chunkPosition" );
	m_iWaterTimerUniform = m_xWaterProgram->uniformLocation( "time" );
	m_xWaterProgram->bind();
	m_xWaterProgram->setUniformValue( "atlas", 0 );
	m_xWaterProgram->setUniformValue( "tileCount", 16 );
	m_xWaterProgram->setUniformValue( "tileSize", 16 );
	m_xWaterProgram->setUniformValue( "fogDistance", ( float ) ( VIEW_SIZE*CHUNK_SIZE*CHUNK_SCALE )*3.0f );
	glUniform3fv( m_xWaterProgram->uniformLocation( "fogColor" ), 1, glm::value_ptr( skyColor ) );
	//m_xWaterProgram->setUniformValue("fogColor", skyColor);
	m_xWaterProgram->release();

	m_xAtlas = std::make_unique<OpenGLTexture>( "textures/atlas.png" );
	m_xAtlas->setMagnificationFilter( OpenGLTexture::Nearest );

	m_pOglBuffers = new Buffer[ VBO_NUMBER ];
	for( int i = 0; i < VBO_NUMBER; ++i )
	{
		Buffer* buffer = m_pOglBuffers + i;
		buffer->opaqueCount = 0;
		buffer->waterCount = 0;
		buffer->draw = false;

		buffer->iBufferOffset = -1;
		buffer->iAllocatedSize = 0;
	}

	glGenBuffers( 1, &m_iVbo );
	m_pVao = new OpenGLVertexArrayObject();
	m_pVao->create();
	m_pVao->bind();
	glBindBuffer( GL_ARRAY_BUFFER, m_iVbo );
	glEnableVertexAttribArray( m_iPosAttr );
	glVertexAttribIPointer( m_iPosAttr, 1, GL_UNSIGNED_INT, 0, 0 );
	glBufferStorage( GL_ARRAY_BUFFER, GRAPHIC_BUFFER_SIZE, NULL, GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT );
	m_pVao->release();

	m_bIsInit = true;

	m_oThread = std::thread( &ChunkManager::run, this );
}

void ChunkManager::destroy( GameWindow* /*gl*/ )
{
	m_bNeedRegen = false;
	m_oThread.join();

	m_bIsInit = false;

	m_pVao->destroy();
	delete m_pVao;
	glDeleteBuffers( 1, &m_iVbo );

	// On nettoie les ressources opengl
	for( int i = 0; i < VBO_NUMBER; ++i )
	{
		Buffer* buffer = m_pOglBuffers + i;
	}

	// On supprime les tableaux
	delete[] m_pOglBuffers;
	m_pOglBuffers = nullptr;
	// On supprime l'atlas et le shader
	m_xAtlas = nullptr;
	m_xProgram = nullptr;
	m_xWaterProgram = nullptr;
}

void ChunkManager::update( GameWindow* gl )
{
	m_iChunkToDrawCount = 0;
	Camera& oCamera = gl->getCamera();
	if( m_bIsInit )
	{

		glm::vec3 camPos = gl->getCamera().getPosition();

		Coords chunkHere = GetChunkPosFromVoxelPos( GetVoxelPosFromWorldPos( camPos ) );

		float camX = gl->getCamera().getPosition().x;
		float camY = gl->getCamera().getPosition().y;
		float camZ = gl->getCamera().getPosition().z;

		if( ( chunkHere.i != m_oCurrentChunk.i ) || ( chunkHere.k != m_oCurrentChunk.k ) || m_bFirstUpdate )
		{
			m_oCurrentChunk = chunkHere;

			int currentMapIndex = m_iMapIndex;
			int nextMapIndex = ( currentMapIndex + 1 ) % 2;

			memset( m_pChunksMapping[ nextMapIndex ], 0, sizeof( Chunk* ) * CHUNK_NUMBER );
			m_oNextFreeBuffer.clear();
			for( int i = 0; i < CHUNK_NUMBER; ++i )
			{
				Chunk* chunk = m_pChunks + i;

				// Les chunks hors de la vue
				if( ( std::abs( chunk->i - chunkHere.i ) > VIEW_SIZE ) ||
					( std::abs( chunk->k - chunkHere.k ) > VIEW_SIZE ) )
				{
					m_oMutexGenerateQueue.lock();
					for( int j = 0; j < ( int ) m_oToGenerateChunkData.size(); ++j )
					{
						if( m_oToGenerateChunkData[ j ] == chunk )
						{
							m_oToGenerateChunkData[ j ] = nullptr;
							break;
						}
					}
					chunk->inQueue = false;
					m_oMutexGenerateQueue.unlock();
					chunk->visible = false;
					chunk->ready = false;
					chunk->isDirty = true;
					chunk->isLightDirty = true;
					chunk->onlyAir = true;
					chunk->differsFromDisk = false;
					chunk->generated = false;
					m_oNextFreeBuffer.push_back( chunk );
				}
				else
				{
					int newIndex = getArrayIndex( chunk->i, chunk->j, chunk->k, chunkHere );
					m_pChunksMapping[ nextMapIndex ][ newIndex ] = chunk;
				}
			}
			Chunk** newChunkArray = m_pChunksMapping[ nextMapIndex ];
			for( int i = 0; i < CHUNK_NUMBER; ++i )
			{
				if( newChunkArray[ i ] == nullptr )
				{
					newChunkArray[ i ] = m_oNextFreeBuffer.back();
					m_oNextFreeBuffer.pop_back();
					Coords c = getChunkCoords( i, chunkHere );
					newChunkArray[ i ]->i = c.i;
					newChunkArray[ i ]->j = c.j;
					newChunkArray[ i ]->k = c.k;

					m_oMutexGenerateQueue.lock();
					m_oToGenerateChunkData.push_back( newChunkArray[ i ] );
					newChunkArray[ i ]->inQueue = true;
					m_oMutexGenerateQueue.unlock();
				}
			}
			m_oChunksCenter[ nextMapIndex ] = chunkHere;
			m_iMapIndex = nextMapIndex;
		}
		int iChunkToSort = 0;
		int remesh_count = 0;
		glBindBuffer( GL_ARRAY_BUFFER, m_iVbo );
		for( int i = 0; i < CHUNK_NUMBER; ++i )
		{
			Chunk* chunk = m_pChunks + i;
			bool remesh = false;
			if( ( chunk->isDirty || chunk->isLightDirty ) && ( remesh_count < MAX_REMESH_PER_UPDATE ) && ( chunk->vboIndex != -1 ) && chunk->generated )
			{
				chunk->isDirty = false;
				chunk->isLightDirty = false;
				remesh = true;
			}

			// Regénération du mesh pour ce chunk
			if( remesh )
			{
				remesh_count++;
				Buffer* buffer = m_pOglBuffers + chunk->vboIndex;
				Voxel *data = chunk->data;
				unsigned int totalCount = 0;
				if( data != nullptr )
				{
					totalCount = m_pMeshGenerator->generate( data, Coords { chunk->i, chunk->j, chunk->k }, buffer, m_pTempVertexData );
				}
				if( totalCount > 0 )
				{
					// On alloue un buffer plus grand si besoin
					if( totalCount > ( buffer->opaqueCount + buffer->waterCount ) )
					{
						if( buffer->iBufferOffset == -1 )
							buffer->iBufferOffset = m_oBuddyAllocator.Allocate( totalCount * sizeof( GLuint ) );
						else
							buffer->iBufferOffset = m_oBuddyAllocator.ReAlloc( buffer->iBufferOffset,
																			   buffer->iAllocatedSize,
																			   totalCount * sizeof( GLuint ) );
						buffer->iAllocatedSize = totalCount * sizeof( GLuint );
						if( buffer->iBufferOffset == -1 )
							printf( "Could not allocate %d bytes of memory for buffer %d\r\n", totalCount * sizeof( GLuint ), chunk->vboIndex );
					}
					if( buffer->iBufferOffset != -1 )
					{
						void* pDataMap = glMapBufferRange( GL_ARRAY_BUFFER, buffer->iBufferOffset, totalCount * sizeof( GLuint ), GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT );
						if( pDataMap != nullptr )
						{
							memcpy( pDataMap, m_pTempVertexData, totalCount * sizeof( GLuint ) );
							glUnmapBuffer( GL_ARRAY_BUFFER );
						}
						else
							printf("Error while mapping vbo\r\n");
						chunk->ready = true;
						chunk->visible = true;
						buffer->opaqueCount = buffer->toUpOpaqueCount;
						buffer->waterCount = buffer->toUpWaterCount;
						buffer->draw = true;
						remesh = false;
					}
				}
				else
				{
					buffer->draw = true;
					chunk->ready = true;
					chunk->visible = true;
				}
			}
			if( chunk->ready && chunk->generated && chunk->visible && !chunk->onlyAir )
			{
				//bool bInFrustum = oCamera.boxInFrustum(chunk->i*CHUNK_SIZE*CHUNK_SCALE, chunk->j*CHUNK_SIZE*CHUNK_SCALE, chunk->k*CHUNK_SIZE*CHUNK_SCALE, CHUNK_SIZE*CHUNK_SCALE);
				bool bInFrustum = oCamera.sphereInFrustum(
					glm::vec3( chunk->i*CHUNK_SIZE*CHUNK_SCALE + CHUNK_SIZE*CHUNK_SCALE / 2.0f,
							   chunk->j*CHUNK_SIZE*CHUNK_SCALE + CHUNK_SIZE*CHUNK_SCALE / 2.0f,
							   chunk->k*CHUNK_SIZE*CHUNK_SCALE + CHUNK_SIZE*CHUNK_SCALE / 2.0f ),
					CHUNK_SIZE*CHUNK_SCALE );
				if( bInFrustum )
				{
					int dx = ( int ) camX - chunk->i*CHUNK_SIZE*CHUNK_SCALE;
					int dy = ( int ) camY - chunk->j*CHUNK_SIZE*CHUNK_SCALE;
					int dz = ( int ) camZ - chunk->k*CHUNK_SIZE*CHUNK_SCALE;
					chunk->distanceFromCamera = dx * dx + dy * dy + dz * dz;

					m_pChunkToSort[ iChunkToSort ].iDistance = chunk->distanceFromCamera;
					m_pChunkToSort[ iChunkToSort ].iChunkIndex = i;
					++iChunkToSort;
				}
			}
		}

		std::sort( m_pChunkToSort, m_pChunkToSort + iChunkToSort, []( const ChunkSortData& i, const ChunkSortData& j )->bool {
			return i.iDistance < j.iDistance;
		} );

		for( int i = 0; i < iChunkToSort; ++i )
		{
			m_pChunkToDraw[ m_iChunkToDrawCount++ ] = m_pChunks + m_pChunkToSort[ i ].iChunkIndex;
		}

		m_bFirstUpdate = false;
	}
}

int ChunkManager::getArrayIndex( Coords chunkPos, Coords center )
{
	int i = ( chunkPos.i - center.i ) + VIEW_SIZE;
	int k = ( chunkPos.k - center.k ) + VIEW_SIZE;
	return ( i + ( ( k + ( chunkPos.j * FULL_VIEW_SIZE ) ) * FULL_VIEW_SIZE ) );
}

int ChunkManager::getArrayIndex( int i, int j, int k, Coords center )
{
	int newI = ( i - center.i ) + VIEW_SIZE;
	int newK = ( k - center.k ) + VIEW_SIZE;
	return ( newI + ( ( newK + ( j * FULL_VIEW_SIZE ) ) * FULL_VIEW_SIZE ) );
}

Coords ChunkManager::getChunkCoords( int index, Coords center )
{
	int layerSize = ( FULL_VIEW_SIZE * FULL_VIEW_SIZE );

	int j = div_floor( index, layerSize );
	int indexInLayer = index % layerSize;
	int i = ( indexInLayer % FULL_VIEW_SIZE ) - VIEW_SIZE + center.i;
	int k = ( indexInLayer / FULL_VIEW_SIZE ) - VIEW_SIZE + center.k;

	return { i, j, k };
}

void ChunkManager::draw( GameWindow* gl )
{
	if( m_bIsInit )
	{
		m_xProgram->bind();

		glm::mat4x4 mat = gl->getCamera().getViewProjMatrix();
		glm::mat4x4 scale = glm::scale( glm::mat4x4(), glm::vec3( ( float ) CHUNK_SCALE, ( float ) CHUNK_SCALE, ( float ) CHUNK_SCALE ) );
		m_xProgram->setUniformValue( m_iMatrixUniform, mat * scale );
		m_xAtlas->bind( 0 );

		m_pVao->bind();
		for( int i = m_iChunkToDrawCount - 1; i >= 0; --i )
		{
			Chunk* chunk = m_pChunkToDraw[ i ];
			Buffer* buffer = m_pOglBuffers + chunk->vboIndex;

			if( chunk->generated && buffer->draw && buffer->opaqueCount > 0 && buffer->iBufferOffset >= 0 )
			{
				glUniform3fv( m_iChunkPosUniform, 1, glm::value_ptr( glm::vec3( ( float ) ( chunk->i * CHUNK_SIZE ),
																			   ( float ) ( chunk->j * CHUNK_SIZE ),
																			   ( float ) ( chunk->k * CHUNK_SIZE ) ) ) );
				glDrawArrays( GL_TRIANGLES, buffer->iBufferOffset / sizeof( GLuint ), buffer->opaqueCount );
			}
		}
		glDisable( GL_CULL_FACE );
		m_xWaterProgram->bind();
		m_xWaterProgram->setUniformValue( m_iWaterMatrixUniform, mat * scale );
		m_xWaterProgram->setUniformValue( m_iWaterTimerUniform, ( float ) m_oAnimationTime.elapsed() );
		for( int i = m_iChunkToDrawCount - 1; i >= 0; --i )
		{
			Chunk* chunk = m_pChunkToDraw[ i ];
			Buffer* buffer = m_pOglBuffers + chunk->vboIndex;
			if( chunk->generated && buffer->draw && buffer->waterCount > 0 && buffer->iBufferOffset >= 0 )
			{
				glUniform3fv( m_iWaterChunkPosUniform, 1, glm::value_ptr( glm::vec3( ( float ) ( chunk->i * CHUNK_SIZE ),
																					( float ) ( chunk->j * CHUNK_SIZE ),
																					( float ) ( chunk->k * CHUNK_SIZE ) ) ) );
				glDrawArrays( GL_TRIANGLES, buffer->iBufferOffset / sizeof( GLuint ) + buffer->opaqueCount, buffer->waterCount );
			}
		}
		m_pVao->release();
		glEnable( GL_CULL_FACE );
	}
}

Voxel* ChunkManager::getBufferAdress( int index )
{
	return index != -1 ? m_pChunkBuffers + ( index * CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE ) : nullptr;
}

Chunk* ChunkManager::getChunk( Coords pos )
{
	Chunk* lastChunk = m_pLastChunk;
	if( lastChunk != nullptr )
	{
		Coords c = { lastChunk->i, lastChunk->j, lastChunk->k };
		if( c == pos )
			return lastChunk;
	}

	int mapIndex = m_iMapIndex;
	Coords currentPos = m_oChunksCenter[ mapIndex ];
	if( ( std::abs( pos.i - currentPos.i ) > VIEW_SIZE ) ||
		( std::abs( pos.k - currentPos.k ) > VIEW_SIZE ) )
	{
		return nullptr;
	}

	if( pos.j < 0 || pos.j >= WORLD_HEIGHT )
		return nullptr;

	int i = ( pos.i - currentPos.i ) + VIEW_SIZE;
	int k = ( pos.k - currentPos.k ) + VIEW_SIZE;
	int index = i + ( ( k + ( pos.j * FULL_VIEW_SIZE ) ) * FULL_VIEW_SIZE );

	m_pLastChunk = m_pChunksMapping[ mapIndex ][ index ];
	return m_pChunksMapping[ mapIndex ][ index ];
}

Chunk* ChunkManager::getChunk( int i, int j, int k )
{
	return getChunk( { i, j, k } );
}

void ChunkManager::run()
{

	while( m_bNeedRegen )
	{
		// On génére tous les chunks requis

		if( m_oToGenerateChunkData.size() > 0 )
		{
			m_oMutexGenerateQueue.lock();
			Coords here = m_oCurrentChunk;
			std::sort( m_oToGenerateChunkData.begin(), m_oToGenerateChunkData.end(), [here]( Chunk* c1, Chunk* c2 )->bool {
				if( c1 == nullptr )
					return false;
				else if( c2 == nullptr )
					return true;
				int a = ( c1->i - here.i )*( c1->i - here.i ) + ( c1->k - here.k )*( c1->k - here.k );
				int b = ( c2->i - here.i )*( c2->i - here.i ) + ( c2->k - here.k )*( c2->k - here.k );
				if( a == b )
				{
					return c1->j < c2->j;
				}
				return a > b;
			} );


			Chunk* newChunk = nullptr;
			while( ( newChunk == nullptr ) && ( m_oToGenerateChunkData.size() > 0 ) )
			{
				newChunk = m_oToGenerateChunkData.back();
				m_oToGenerateChunkData.pop_back();
			}
			if( newChunk == nullptr )
				continue;
			m_oMutexGenerateQueue.unlock();

			if( !newChunk->generated )
			{
				// Générer le nouveau chunk et le prendre du DD si déja présent
				Voxel* data = newChunk->data;
				if( data != nullptr )
				{

					bool skipGeneration = false;
					if( m_oIOManager.chunkExist( Coords { newChunk->i, newChunk->j, newChunk->k } ) )
					{
						m_oIOManager.loadChunk( Coords { newChunk->i, newChunk->j, newChunk->k }, data );
						newChunk->differsFromDisk = !skipGeneration;
					}

					std::set<Coords> modifiedChunks;
					if( !skipGeneration )
					{
						newChunk->onlyAir = m_pChunkGenerator->generateChunk( data, newChunk->i, newChunk->j, newChunk->k, modifiedChunks );
						m_oIOManager.saveChunk( Coords { newChunk->i, newChunk->j, newChunk->k }, data );
						newChunk->differsFromDisk = false;
					}

					m_pLightManager->updateLighting( newChunk );
					modifiedChunks.erase( Coords { newChunk->i, newChunk->j, newChunk->k } );
					for( auto pos : modifiedChunks )
					{
						Chunk* c = getChunk( pos );
						if( c != nullptr )
						{
							if( c->generated )
							{
								m_pLightManager->updateLighting( c );
								Voxel* voxel_data = c->data;
								m_oIOManager.saveChunk( Coords { c->i, c->j, c->k }, voxel_data );
								c->isDirty = true;
								c->differsFromDisk = false;
							}
						}
					}

					newChunk->generated = true;
					newChunk->isDirty = true;
				}
				else
				{
					newChunk->isDirty = true;
				}
			}
			newChunk->inQueue = false;
		}

		// Eviter de faire fondre le cpu dans des boucles vides ;)
		//QThread::msleep(5);
	}
}

Voxel ChunkManager::getVoxel( int x, int y, int z, bool* loaded )
{
	Voxel res = IGNORE_VOXEL;
	if( loaded != nullptr )
	{
		*loaded = true;
	}
	Coords c = GetChunkPosFromVoxelPos( { x, y, z } );
	Chunk* chunk = getChunk( c );
	if( chunk == nullptr )
	{
		if( loaded != nullptr )
		{
			*loaded = false;
		}
		if( y >= WORLD_HEIGHT*CHUNK_SIZE )
			res._light = SUN_LIGHT;

		return res;
	}

	Voxel* voxels = chunk->data;
	if( voxels != nullptr )
	{
		Coords c = GetVoxelRelPos( { x, y, z } );
		res = voxels[ IndexVoxelRelPos( c ) ];
	}
	else if( loaded != nullptr )
	{
		*loaded = false;
	}
	return res;
}

Voxel ChunkManager::getVoxel( Coords c )
{
	return getVoxel( c.i, c.j, c.k );
}

VoxelType ChunkManager::setVoxel( Coords c, VoxelType newType, uint8 light )
{
	return setVoxel( c.i, c.j, c.k, newType, light );
}

VoxelType ChunkManager::setVoxel( int x, int y, int z, VoxelType newType, uint8 light )
{
	VoxelType res = VoxelType::IGNORE_TYPE;

	Chunk* chunk = getChunk( div_floor( x, CHUNK_SIZE ), div_floor( y, CHUNK_SIZE ), div_floor( z, CHUNK_SIZE ) );
	if( chunk == nullptr )
		return res;
	Voxel* voxels = chunk->data;
	if( voxels != nullptr )
	{
		Coords c = GetVoxelRelPos( { x, y, z } );
		int index = IndexVoxelRelPos( c );

		Voxel* v = &voxels[ index ];

		res = v->type;

		if( light != NO_CHANGE )
		{
			v->_light = ( uint8 ) light;
		}

		v->type = newType;

		if( chunk->onlyAir && newType != VoxelType::AIR )
			chunk->onlyAir = false;
	}

	return res;
}

LightManager& ChunkManager::getLightManager()
{
	return *m_pLightManager;
}

VoxelType ChunkManager::placeVoxel( Coords pos, VoxelType type )
{

	std::set<Coords> modifiedChunks;

	VoxelType oldType = getVoxel( pos ).type;

	m_pLightManager->placeVoxel( pos, type, modifiedChunks );

	Chunk* thisChunk = getChunk( GetChunkPosFromVoxelPos( pos ) );

	thisChunk->differsFromDisk = true;

	for( auto coords : modifiedChunks )
	{
		Chunk* chunk = getChunk( coords );
		if( chunk != nullptr )
		{
			chunk->inQueue = true;
			chunk->isLightDirty = true;
		}
	}
	if( thisChunk != nullptr )
	{
		thisChunk->inQueue = true;
		thisChunk->isLightDirty = true;
		thisChunk->isDirty = true;

		if( thisChunk->onlyAir && type != VoxelType::AIR )
			thisChunk->onlyAir = false;
	}

	return oldType;
}

void ChunkManager::removeVoxel( Coords pos )
{

	std::set<Coords> modifiedChunks;
	m_pLightManager->removeVoxel( pos, modifiedChunks );

	Chunk* thisChunk = getChunk( GetChunkPosFromVoxelPos( pos ) );
	thisChunk->differsFromDisk = true;

	for( auto coords : modifiedChunks )
	{
		Chunk* chunk = getChunk( coords );
		if( chunk != nullptr )
		{
			chunk->inQueue = true;
			chunk->isLightDirty = true;
		}
	}

	if( thisChunk != nullptr )
	{
		thisChunk->inQueue = true;
		thisChunk->isLightDirty = true;
		thisChunk->isDirty = true;
	}
}

static int nextPower( int i )
{
	unsigned int iMask = 1 << 31;
	int n = 0;
	while( ( iMask & i ) == 0 && ( n < 32 ) )
	{
		iMask = iMask >> 1;
		n++;
	}
	if( n == 32 )
		return 0;
	if( iMask == i )
		return iMask;

	return iMask << 1;
}

void ChunkManager::printVboMemory()
{
	int iSize = 0;
	int iPowerSize = 0;
	int iCount = 0;

	for( int i = 0; i < CHUNK_NUMBER; ++i )
	{
		Chunk* pChunk = m_pChunks + i;
		Buffer* pBuffer = m_pOglBuffers + pChunk->vboIndex;

		if( pChunk->generated && pBuffer->draw )
		{
			Buffer* pBuffer = m_pOglBuffers + pChunk->vboIndex;
			iSize += pBuffer->opaqueCount + pBuffer->waterCount;
			iPowerSize += nextPower( ( pBuffer->opaqueCount + pBuffer->waterCount ) * 4 );
			int iTmpSize = ( pBuffer->opaqueCount + pBuffer->waterCount ) * 4;
			//std::cout << "Chunk Size : " << iTmpSize << " octets " << nextPower(iTmpSize) << std::endl;
			iCount++;
		}
	}

	iSize *= 4;

	std::cout << "Total Size : " << iSize << " octets" << std::endl;
	std::cout << "Total Size power of two : " << iPowerSize << " octets" << std::endl;
	std::cout << "Average size : " << iSize / iCount << " octets" << std::endl;
}
