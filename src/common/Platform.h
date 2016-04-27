#ifndef PLATFORM_H
#define PLATFORM_H

#ifdef WIN32

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#undef GetCurrentTime
#undef ARRAYSIZE

#define MAX_PATH_LENGTH MAX_PATH

#define strcasecmp _stricmp
#define strncasecmp _strnicmp

#define getcwd _getcwd
#define setcwd _setcwd

#else

#error "Not implemented"

#define MAX_PATH_LENGTH PATH_MAX

#endif

#endif //PLATFORM_H
