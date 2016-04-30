#include <cassert>
#include <utility>

#include "ByteSwap.h"

uint8_t SwapByte( uint8_t value )
{
	return value;
}

int16_t SwapShort( int16_t value )
{
	int16_t result = 
		( ( value << 8 ) & 0xFF00 ) | 
		( ( value >> 8 ) & 0xFF );

	return result;
}

int32_t SwapLong( int32_t value )
{
	const int8_t b1 = value & 0xFF;
	const int8_t b2 = ( value >> 8 ) & 0xFF;
	const int8_t b3 = ( value >> 16 ) & 0xFF;
	const int8_t b4 = ( value >> 24 ) & 0xFF;

	return 
		( b1 << 24 ) | 
		( b2 << 16 ) | 
		( b3 << 8 ) | 
		b4;
}

int64_t SwapLongLong( int64_t value )
{
	const int8_t b1 = value & 0xFF;
	const int8_t b2 = ( value >> 8 ) & 0xFF;
	const int8_t b3 = ( value >> 16 ) & 0xFF;
	const int8_t b4 = ( value >> 24 ) & 0xFF;
	const int8_t b5 = ( value >> 32 ) & 0xFF;
	const int8_t b6 = ( value >> 40 ) & 0xFF;
	const int8_t b7 = ( value >> 48 ) & 0xFF;
	const int8_t b8 = ( value >> 56 ) & 0xFF;

	return 
		( static_cast<int64_t>( b1 ) << 56 ) | 
		( static_cast<int64_t>( b2 ) << 48 ) | 
		( static_cast<int64_t>( b3 ) << 40 ) | 
		( static_cast<int64_t>( b4 ) << 32 ) | 
		( b5 << 24 ) | 
		( b6 << 16 ) | 
		( b7 << 8 ) | 
		b8;
}

float SwapFloat( float value )
{
	static_assert( sizeof( float ) == 4, "Byte swapping does not support floats != 4 bytes large!" );

	union
	{
		float value;
		uint8_t bytes[ sizeof( float ) ];
	} input, result;

	input.value = value;

	result.bytes[ 0 ] = input.bytes[ 3 ];
	result.bytes[ 1 ] = input.bytes[ 2 ];
	result.bytes[ 2 ] = input.bytes[ 1 ];
	result.bytes[ 3 ] = input.bytes[ 0 ];

	return result.value;
}

double SwapDouble( double value )
{
	static_assert( sizeof( double ) == 8, "Byte swapping does not support double != 8 bytes large!" );

	union
	{
		double value;
		uint8_t bytes[ sizeof( double ) ];
	} input, result;

	input.value = value;

	result.bytes[ 0 ] = input.bytes[ 7 ];
	result.bytes[ 1 ] = input.bytes[ 6 ];
	result.bytes[ 2 ] = input.bytes[ 5 ];
	result.bytes[ 3 ] = input.bytes[ 4 ];
	result.bytes[ 4 ] = input.bytes[ 3 ];
	result.bytes[ 5 ] = input.bytes[ 2 ];
	result.bytes[ 6 ] = input.bytes[ 1 ];
	result.bytes[ 7 ] = input.bytes[ 0 ];

	return result.value;
}

uint8_t* SwapBytes( uint8_t* pBytes, const size_t uiSize )
{
	assert( pBytes );

	for( size_t uiIndex = 0; uiIndex < ( uiSize / 2 ); ++uiIndex )
	{
		std::swap( pBytes[ uiIndex ], pBytes[ uiSize - uiIndex - 1 ] );
	}

	return pBytes;
}