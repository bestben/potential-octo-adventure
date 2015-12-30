#pragma once

#include <cmath>

class BuddyAllocator {

public:
	BuddyAllocator( int iSize );
	~BuddyAllocator();

	int		Allocate( int iSize );
	int		ReAlloc( int iOffset, int iOldSize, int iNewSize );
	void	Free( int iOffset, int iSize );

private:
	static int NextPowerOfTwo( int i );

	int		GetFreeBlock( int iLevel );
	void	SplitBlock( int iLevel );
	void	RemoveFromFreelist( int iLevel, int iBlock );
	void	TryMerge( int iLevel, int iBlock );
	int		GetLevel( int iSize );

	char*	m_pAllocationTable;
	int*	m_pFreeList;
	int*	m_pFreeListHead;

	int		m_iLevelCount;
	int		m_iSize; // Total size of the allocator ( power of two )
	int		m_iBlockCount;
};