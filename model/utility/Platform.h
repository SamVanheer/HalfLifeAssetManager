#ifndef PLATFORM_H
#define PLATFORM_H

#ifdef WIN32

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#define MAX_PATH_LENGTH MAX_PATH

#else

#error "Not implemented"

#define MAX_PATH_LENGTH PATH_MAX

#endif

#endif //PLATFORM_H