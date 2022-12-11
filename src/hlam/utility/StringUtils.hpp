#pragma once

#include <cstring>

inline const char* UTIL_CopyString(char* dest, const char* source, std::size_t destSizeInBytes)
{
	if (destSizeInBytes > 0)
	{
		strncpy(dest, source, destSizeInBytes - 1);
		dest[destSizeInBytes - 1] = '\0';
	}

	return dest;
}

template<std::size_t Size>
inline const char* UTIL_CopyString(char (&dest)[Size], const char* source)
{
	return UTIL_CopyString(dest, source, sizeof(dest));
}
