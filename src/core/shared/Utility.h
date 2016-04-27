#ifndef COMMON_UTILITY_H
#define COMMON_UTILITY_H

#include <algorithm>
#include <cstdint>
#include <cstring>

#include "Platform.h"

/**
*	Utility functionality.
*/

/**
*	Checks if a printf operation was successful
*/
inline bool PrintfSuccess( const int iRet, const size_t uiBufferSize )
{
	return iRet >= 0 && static_cast<size_t>( iRet ) < uiBufferSize;
}

/**
*	Taken from MSVC string hash.
*/
inline size_t StringHash( const char* const pszString )
{
#if defined(_WIN64)
	static_assert( sizeof( size_t ) == 8, "This code is for 64-bit size_t." );
	const size_t _FNV_offset_basis = 14695981039346656037ULL;
	const size_t _FNV_prime = 1099511628211ULL;

#else /* defined(_WIN64) */
	static_assert( sizeof( size_t ) == 4, "This code is for 32-bit size_t." );
	const size_t _FNV_offset_basis = 2166136261U;
	const size_t _FNV_prime = 16777619U;
#endif /* defined(_WIN64) */

	const size_t _Count = strlen( pszString );

	size_t _Val = _FNV_offset_basis;
	for( size_t _Next = 0; _Next < _Count; ++_Next )
	{	// fold in another byte
		_Val ^= ( size_t ) pszString[ _Next ];
		_Val *= _FNV_prime;
	}
	return ( _Val );
}

template<typename STR>
struct Hash_C_String final : public std::unary_function<STR*, size_t>
{
	std::size_t operator()( STR pszStr ) const
	{
		return StringHash( pszStr );
	}
};

template<typename STR, int ( *COMPARE )( STR lhs, STR rhs ) = strcmp>
struct EqualTo_C_String final
{
	constexpr bool operator()( STR lhs, STR rhs ) const
	{
		return COMPARE( lhs, rhs ) == 0;
	}
};

/**
*	Initializes the random number generator.
*/
void UTIL_InitRandom();

/**
*	Gets a random 32 bit integer number in the range [iLow, iHigh]
*/
int UTIL_RandomLong( int iLow, int iHigh );

/**
*	Gets a random 32 bit floating point number in the range [flLow, flHigh]
*/
float UTIL_RandomFloat( float flLow, float flHigh );

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

/**
*	Returns the current tick time, in milliseconds.
*	@return Tick time, in milliseconds.
*/
long long GetCurrentTick();

/**
*	Gets the current time, in seconds.
*	@return Current time, in seconds.
*/
double GetCurrentTime();

#endif //COMMON_UTILITY_H
