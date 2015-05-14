#pragma once

#include "glm/vec3.hpp"

#define MI_FORCE_INLINE __forceinline

inline float lengthSquare( const glm::vec3& vec ) {
    return (vec.x * vec.x) + (vec.y * vec.y) + (vec.z * vec.z);
}

#ifdef QT_DEBUG
#define MI_ASSERT( x ) if( !(x) ) *((int*)0) = 0;
#else
#define MI_ASSERT( x )
#endif
