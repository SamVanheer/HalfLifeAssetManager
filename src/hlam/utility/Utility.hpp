#pragma once

#include <cstdint>

/**
*	@file
*
*	Utility functionality
*/

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
