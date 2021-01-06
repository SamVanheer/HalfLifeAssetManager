#pragma once

#include <cstdio>
#include <cstring>

#ifdef WIN32

#define strcasecmp _stricmp
#define strncasecmp _strnicmp

#define DLLEXPORT __declspec(dllexport)
#define DLLIMPORT __declspec(dllimport)

#else

#include <strings.h>

#define DLLEXPORT __attribute__((visibility("default")))
#define DLLIMPORT

#endif
