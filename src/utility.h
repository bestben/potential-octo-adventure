#pragma once

#include "glm/vec3.hpp"
#include <atomic>

#ifdef WIN32
#define MI_FORCE_INLINE __forceinline
#else
#define MI_FORCE_INLINE inline
#endif

inline float lengthSquare( const glm::vec3& vec ) {
    return (vec.x * vec.x) + (vec.y * vec.y) + (vec.z * vec.z);
}

#ifdef WIN32
#define CPU_BREAK __asm { int 3 }
#else
#define CPU_BREAK *((char*)0) = 0;
#endif

#ifdef MI_DEBUG
#define MI_ASSERT( x ) if( !(x) ) CPU_BREAK
#else
#define MI_ASSERT( x )
#endif

class SpinLock {
public:
	SpinLock(std::atomic<int>& lock) : m_lock(lock) {
		int old = false;
		while ( !m_lock.compare_exchange_weak(old, 1) );
	}

	~SpinLock() {
		m_lock.store(0);
	}

private:
	std::atomic<int>& m_lock;
};

#define MI_SPINLOCK( l ) SpinLock( l )