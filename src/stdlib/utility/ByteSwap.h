#ifndef UTILITY_BYTESWAP_H
#define UTILITY_BYTESWAP_H

#include <cassert>
#include <cstdint>
#include <cstring>
#include <type_traits>

/**
*	@defgroup ByteSwap Byte swapping functions
*
*	@{
*/

/**
*	Swaps a single byte. Effectively does nothing.
*/
uint8_t SwapByte( uint8_t value );

/**
*	Swaps a short.
*/
int16_t SwapShort( int16_t value );

/**
*	Swaps a long.
*/
int32_t SwapLong( int32_t value );

/**
*	Swaps a long long.
*/
int64_t SwapLongLong( int64_t value );

/**
*	Swaps a float.
*/
float SwapFloat( float value );

/**
*	Swaps a double.
*/
double SwapDouble( double value );

/**
*	Swaps an arbitrary size value.
*/
uint8_t* SwapBytes( uint8_t* pBytes, const size_t uiSize );

/**
*	Swaps a value.
*/
template<typename T>
T SwapValue( T value )
{
	return *reinterpret_cast<T*>( SwapBytes( reinterpret_cast<uint8_t*>( &value ), sizeof( T ) ) );
}

template<>
inline uint8_t SwapValue<uint8_t>( uint8_t value )
{
	return SwapByte( value );
}

template<>
inline int16_t SwapValue<int16_t>( int16_t value )
{
	return SwapShort( value );
}

template<>
inline int32_t SwapValue<int32_t>( int32_t value )
{
	return SwapLong( value );
}

template<>
inline int64_t SwapValue<int64_t>( int64_t value )
{
	return SwapLongLong( value );
}

template<>
inline float SwapValue<float>( float value )
{
	return SwapFloat( value );
}

template<>
inline double SwapValue<double>( double value )
{
	return SwapDouble( value );
}

/**
*	Calls the correct byte swapping functions for the current platform.
*/
template<const bool LITTLE = IS_LITTLE_ENDIAN>
class ByteSwap final
{
public:
	/**
	*	Returns whether this is a little endian system or not.
	*/
	static constexpr bool IsLittleEndian()
	{
		assert( !"Undefined byte swap order!" );
		return false;
	}

	/**
	*	Byte swaps the given value to little endian.
	*/
	template<typename T>
	static T LittleValue( T value )
	{
		assert( !"Undefined byte swap order!" );
	}

	/**
	*	Byte swaps the given value to big endian.
	*/
	template<typename T>
	static T BigValue( T value )
	{
		assert( !"Undefined byte swap order!" );
	}
};

template<>
class ByteSwap<true> final
{
public:

	static constexpr bool IsLittleEndian()
	{
		return true;
	}

	template<typename T>
	static T LittleValue( T value )
	{
		return value;
	}

	template<typename T>
	static T BigValue( T value )
	{
		return SwapValue( value );
	}
};

template<>
class ByteSwap<false> final
{
public:

	static constexpr bool IsLittleEndian()
	{
		return false;
	}

	template<typename T>
	static T LittleValue( T value )
	{
		return SwapValue( value );
	}

	template<typename T>
	static T BigValue( T value )
	{
		return value;
	}
};

/**
*	On a little endian system, this will byte swap values. On a big endian system, no swapping occurs.
*/
template<typename T>
T LittleValue( T value )
{
	return ByteSwap<>::LittleValue( value );
}

/**
*	Byteswap for enums.
*	@see LittleValue
*/
template<typename T>
T LittleEnumValue( T value )
{
	return static_cast<T>( ByteSwap<>::LittleValue( static_cast<typename std::underlying_type<T>::type>( value ) ) );
}

/**
*	On a big endian system, this will byte swap values. On a little endian system, no swapping occurs.
*/
template<typename T>
T BigValue( T value )
{
	return ByteSwap<>::BigValue( value );
}

/**
*	Byteswap for enums.
*	@see BigValue
*/
template<typename T>
T BigEnumValue( T value )
{
	return static_cast<T>( ByteSwap<>::BigValue( static_cast<typename std::underlying_type<T>::type>( value ) ) );
}

/** @} */

#endif //UTILITY_BYTESWAP_H