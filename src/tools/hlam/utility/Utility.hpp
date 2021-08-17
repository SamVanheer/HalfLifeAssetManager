#pragma once

#include <algorithm>
#include <cstdint>

/**
*	Utility functionality.
*/

/**
*	Initializes the random number generator.
*/
void UTIL_InitRandom();

/**
*	Gets a random 32 bit integer number in the range [iLow, iHigh]
*/
int UTIL_RandomLong(int iLow, int iHigh);

/**
*	Gets a random 32 bit floating point number in the range [flLow, flHigh]
*/
float UTIL_RandomFloat(float flLow, float flHigh);

/**
*	Returns a 1 bit at the given position.
*/
inline constexpr int32_t Bit(const size_t shift)
{
	return static_cast<int32_t>(1 << shift);
}

/**
*	Returns a 1 bit at the given position.
*	64 bit variant.
*/
inline constexpr int64_t Bit64(const size_t shift)
{
	return static_cast<int64_t>(static_cast<int64_t>(1) << shift);
}

template<typename TFlags, typename TFlag>
TFlags SetFlags(TFlags flags, TFlag flag, bool set = true)
{
	const TFlags convertedValue{static_cast<TFlags>(flag)};

	if (set)
	{
		return flags | convertedValue;
	}
	else
	{
		return flags & ~convertedValue;
	}
}
