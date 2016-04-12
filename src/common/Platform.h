#ifndef PLATFORM_H
#define PLATFORM_H

#ifdef WIN32

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#undef GetCurrentTime
#undef ARRAYSIZE

#define MAX_PATH_LENGTH MAX_PATH

#define strcasecmp _stricmp
#define strncasecmp _strnicmp

#else

#error "Not implemented"

#define MAX_PATH_LENGTH PATH_MAX

#endif

template<typename T, const size_t SIZE>
constexpr inline size_t _ArraySizeof( T ( & )[ SIZE ] )
{
	return SIZE;
}


#ifndef ARRAYSIZE
#define ARRAYSIZE _ArraySizeof
#endif

#define MAX_BUFFER_LENGTH 512

long long GetCurrentTick();

double GetCurrentTime();

#endif //PLATFORM_H