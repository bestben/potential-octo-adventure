#pragma once

#include "glm/vec3.hpp"

#define MI_FORCE_INLINE __forceinline

inline float lengthSquare( const glm::vec3& vec ) {
    return (vec.x * vec.x) + (vec.y * vec.y) + (vec.z * vec.z);
}

#ifdef WIN32
#define CPU_BREAK __asm { int 3 }
#elif
#define CPU_BREAK *((char*)0) = 0;
#endif

#ifdef MI_DEBUG
#define MI_ASSERT( x ) if( !(x) ) CPU_BREAK
#else
#define MI_ASSERT( x )
#endif
