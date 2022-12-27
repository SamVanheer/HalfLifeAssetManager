#pragma once

#include <cstring>

#ifdef WIN32

#define strcasecmp _stricmp
#define strncasecmp _strnicmp

#else

#include <strings.h>

#endif
