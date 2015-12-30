#include "BuddyAllocator.h"
#include "../utility.h"
#include <cstring>
#include <algorithm>

#define MINIMUM_BLOCK_SIZE 1024

//Block indices
//
//-----------------------------------------------------------------------------------------------------------------------------
//|																0																|
//-----------------------------------------------------------------------------------------------------------------------------
//|								1								|								2								|
//-----------------------------------------------------------------------------------------------------------------------------
//|				3				|				4				|				5				|				6				|
//-----------------------------------------------------------------------------------------------------------------------------
//|		7		|		8		|		9		|		10		|		11		|		12		|		13		|		14		|
//-----------------------------------------------------------------------------------------------------------------------------
//|	15	|	16	|	17	|	18	|	19	|	20	|	21	|	22	|	23	|	24	|	25	|	26	|	27	|	28	|	29	|	30	|
//-----------------------------------------------------------------------------------------------------------------------------

BuddyAllocator::BuddyAllocator( int iSize )
{
	m_iSize = std::max( MINIMUM_BLOCK_SIZE, NextPowerOfTwo( iSize ) );
	int iMaxLevel = 0;
	for (int i = m_iSize; i > MINIMUM_BLOCK_SIZE; i = i >> 1, ++iMaxLevel) {}
	m_iLevelCount = iMaxLevel + 1;
	m_iBlockCount = (1 << m_iLevelCount) - 1;

	m_pAllocationTable = new char[ m_iBlockCount / 16 + 1 ];
	m_pFreeList = new int[ m_iBlockCount ];
	m_pFreeListHead = new int[ m_iLevelCount ];
	memset( m_pAllocationTable, 0, m_iBlockCount / 16 + 1 );
	int* pPtr = m_pFreeList;
	for( int i = 0; i < m_iBlockCount; ++i )
		m_pFreeList[ i ] = -1;
	for( int i = 0; i < m_iLevelCount; ++i )
		m_pFreeListHead[ i ] = -1;
	m_pFreeListHead[ 0 ] = 0;
}

BuddyAllocator::~BuddyAllocator()
{
	delete[] m_pAllocationTable;
	delete[] m_pFreeList;
	delete[] m_pFreeListHead;
}

int BuddyAllocator::Allocate( int iAllocSize )
{
	if ( iAllocSize <= 0 && iAllocSize > m_iSize )
		return -1;

	int iLevelNeeded = 0;
	int iNeededPowerSize = m_iSize;
	while( ( iAllocSize < ( iNeededPowerSize >> 1 ) ) && ( iNeededPowerSize > MINIMUM_BLOCK_SIZE ) )
	{
		iLevelNeeded++;
		iNeededPowerSize = iNeededPowerSize >> 1;
	}

	MI_ASSERT( iLevelNeeded < m_iLevelCount );
	int iBlock = GetFreeBlock( iLevelNeeded ); // Global index of the block all the blocks
	if( iBlock == -1 )
		return -1;
	int iBlockIndexAtLevel = iBlock - ( 1 << iLevelNeeded ) + 1; // Index of the block in the current level
	int iBlockSizeAtLevel = m_iSize / ( 1 << iLevelNeeded );
	int iBlockOffset = iBlockIndexAtLevel * iBlockSizeAtLevel;
	MI_ASSERT( iBlockOffset < m_iSize );

	return iBlockOffset;
}

int BuddyAllocator::ReAlloc( int iOffset, int iOldSize, int iNewSize )
{
	MI_ASSERT( iOffset >= 0 && iOffset < m_iSize );

	int iLevel = GetLevel( iOldSize );
	int iBlockSizeAtLevel = m_iSize / ( 1 << iLevel );
	int iBlockSizeAtNextLevel = m_iSize / ( 1 << (iLevel + 1) );
	if( iNewSize <= iBlockSizeAtLevel && iNewSize > iBlockSizeAtNextLevel ) // the new size fit this level block size
		return iOffset;
	// the new size is smaller than the block size at next level or bigger than this level => free then alloc new block
	Free( iOffset, iOldSize );
	return Allocate( iNewSize );
}

void BuddyAllocator::Free( int iOffset, int iSize )
{
	int iPowerSize = NextPowerOfTwo( iSize );
	int iLevel = 0;
	for( int i = m_iSize; i > MINIMUM_BLOCK_SIZE && i > iPowerSize; i = i >> 1, ++iLevel ) { }

	int iBlockSizeAtLevel = m_iSize / ( 1 <<  iLevel );
	MI_ASSERT( iOffset % iBlockSizeAtLevel == 0 );
	int iBlockIndexAtLevel = iOffset / iBlockSizeAtLevel;
	int iBlock = iBlockIndexAtLevel + ( 1 << iLevel ) - 1;

	TryMerge( iLevel, iBlock );
}

int BuddyAllocator::GetFreeBlock( int iLevel )
{
	MI_ASSERT( iLevel >= 0 );

	int iBlock = m_pFreeListHead[ iLevel ];
	if( iBlock == -1 )
		SplitBlock( iLevel - 1 );
	iBlock = m_pFreeListHead[ iLevel ];
	if( iBlock == -1 )
		return -1;

	// Remove this block from the freelist
	m_pFreeListHead[ iLevel ] = m_pFreeList[ iBlock ];
	m_pFreeList[ iBlock ] = -1;
	// Mark block as allocated
	int iMask = 1 << (((iBlock + 1) / 2) % 8);
	MI_ASSERT( iBlock < m_iBlockCount );
	m_pAllocationTable[ ( iBlock + 1 ) / 16 ] ^= iMask;

	return iBlock;
}

void BuddyAllocator::SplitBlock( int iLevel )
{
	MI_ASSERT( iLevel >= 0 );
	MI_ASSERT( iLevel < ( m_iLevelCount - 1 ) );

	int iBlock = m_pFreeListHead[ iLevel ];
	if( iLevel > 0 && iBlock == -1 )
		SplitBlock( iLevel - 1 );
	iBlock = m_pFreeListHead[ iLevel ];
	if( iBlock == -1 )
		return;
	// Add new blocks to the freelist
	int iNewBlock = iBlock * 2 + 1;
	m_pFreeList[ iNewBlock + 1 ] = m_pFreeListHead[ iLevel + 1 ];
	m_pFreeList[ iNewBlock ] = iNewBlock + 1;
	m_pFreeListHead[ iLevel + 1 ] = iNewBlock;
	// Remove this block from the freelist
	m_pFreeListHead[ iLevel ] = m_pFreeList[ iBlock ];
	m_pFreeList[ iBlock ] = -1;
	// Mark block as allocated
	int iMask = 1 << ( ( ( iBlock + 1 ) / 2 ) % 8 );
	MI_ASSERT( iBlock < m_iBlockCount );
	m_pAllocationTable[ ( iBlock + 1 ) / 16 ] ^= iMask;
}

void BuddyAllocator::RemoveFromFreelist( int iLevel, int iBlock )
{
	MI_ASSERT( iLevel < m_iLevelCount );
	if( m_pFreeListHead[ iLevel ] == iBlock )
	{
		int iNextBlock = m_pFreeList[ iBlock ];
		m_pFreeList[ iBlock ] = -1;
		m_pFreeListHead[ iLevel ] = iNextBlock;
	}
	else
	{
		int iPreviousBlock = m_pFreeListHead[ iLevel ];
		int iCurrentBlock = m_pFreeList[ iPreviousBlock ];
		while( iCurrentBlock != -1 && iCurrentBlock != iBlock )
		{
			iPreviousBlock = iCurrentBlock;
			iCurrentBlock = m_pFreeList[ iCurrentBlock ];
		}
		if( iCurrentBlock == iBlock )
		{
			m_pFreeList[ iPreviousBlock ] = m_pFreeList[ iBlock ];
			m_pFreeList[ iBlock ] = -1;
		}
	}
}

void BuddyAllocator::TryMerge( int iLevel, int iBlock )
{
	// Get the other block state
	int iMask = 1 << ( ( ( iBlock + 1 ) / 2 ) % 8 );
	MI_ASSERT( iBlock < m_iBlockCount );
	int iBlockState = m_pAllocationTable[ ( iBlock + 1 ) / 16 ] & iMask;
	m_pAllocationTable[ ( iBlock + 1 ) / 16 ] ^= iMask; // Change the state of the block pair
	if( iBlock == 0 || iBlockState == 0 ) // Other block is already allocated => only add this block
	{
		m_pFreeList[ iBlock ] = m_pFreeListHead[ iLevel ];
		m_pFreeListHead[ iLevel ] = iBlock;
	}
	else // Merge the blocks
	{
		int iOtherBlock;
		if( iBlock % 2 == 0 )
			iOtherBlock = iBlock - 1;
		else
			iOtherBlock = iBlock + 1;
		RemoveFromFreelist( iLevel, iOtherBlock );
		int iParentBlock = ( iBlock - 1 ) / 2;
		TryMerge( iLevel - 1, iParentBlock );
	}
}

int BuddyAllocator::NextPowerOfTwo( int i )
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

int BuddyAllocator::GetLevel( int iSize )
{
	int iPowerSize = NextPowerOfTwo( iSize );
	int iLevel = 0;
	for( int i = m_iSize; i > MINIMUM_BLOCK_SIZE && i > iPowerSize; i = i >> 1, ++iLevel ) { }
	return iLevel;
}