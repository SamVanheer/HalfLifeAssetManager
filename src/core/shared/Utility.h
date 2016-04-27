#ifndef COMMON_UTILITY_H
#define COMMON_UTILITY_H

#include <algorithm>
#include <cstdint>

#include "Platform.h"
#include "core/LibHLCore.h"

/**
*	Utility functionality.
*/

/**
*	Initializes the random number generator.
*/
extern "C" HLCORE_API void UTIL_InitRandom();

/**
*	Gets a random 32 bit integer number in the range [iLow, iHigh]
*/
extern "C" HLCORE_API int UTIL_RandomLong( int iLow, int iHigh );

/**
*	Gets a random 32 bit floating point number in the range [flLow, flHigh]
*/
extern "C" HLCORE_API float UTIL_RandomFloat( float flLow, float flHigh );

/**
*	Clamps a given value to a given range.
*/
template<typename T>
T clamp( const T& value, const T& min, const T& max )
{
	return std::max( min, std::min( max, value ) );
}

/**
*	Returns a 1 bit at the given position.
*/
inline constexpr int32_t Bit( const size_t shift )
{
	return static_cast<int32_t>( 1 << shift );
}

/**
*	Returns a 1 bit at the given position.
*	64 bit variant.
*/
inline constexpr int64_t Bit64( const size_t shift )
{
	return static_cast<int64_t>( static_cast<int64_t>( 1 ) << shift );
}

/**
*	Sizeof for array types. Only works for arrays with a known size (stack buffers).
*	@tparam T Array type. Automatically inferred.
*	@tparam SIZE Number of elements in the array.
*	@return Number of elements in the array.
*/
template<typename T, const size_t SIZE>
constexpr inline size_t _ArraySizeof( T( &)[ SIZE ] )
{
	return SIZE;
}

/**
*	Replaces ARRAYSIZE. ARRAYSIZE is defined in some platform specific headers.
*/
#define ARRAYSIZE _ArraySizeof

#define MAX_BUFFER_LENGTH 512

//TODO: this might be better off in a static library;

/**
*	Returns the current tick time, in milliseconds.
*	@return Tick time, in milliseconds.
*/
extern "C" HLCORE_API long long GetCurrentTick();

/**
*	Gets the current time, in seconds.
*	@return Current time, in seconds.
*/
extern "C" HLCORE_API double GetCurrentTime();

#endif //COMMON_UTILITY_H
