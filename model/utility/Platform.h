#ifndef PLATFORM_H
#define PLATFORM_H

#ifdef WIN32

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#define MAX_PATH_LENGTH MAX_PATH

#undef GetCurrentTime

#else

#error "Not implemented"

#define MAX_PATH_LENGTH PATH_MAX

#endif

long long GetCurrentTick();

double GetCurrentTime();

#endif //PLATFORM_H