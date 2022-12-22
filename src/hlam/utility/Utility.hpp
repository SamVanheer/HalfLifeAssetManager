#pragma once

#include <cstdint>

/**
*	@file
*
*	Utility functionality
*/

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

template<class>
inline constexpr bool always_false_v = false;
