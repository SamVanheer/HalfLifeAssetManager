#ifndef CORE_LIBHLCORE_H
#define CORE_LIBHLCORE_H

#include "shared/Platform.h"

#ifdef LIBHLCORE_EXPORTS
#define HLCORE_API DLLEXPORT
#else
#define HLCORE_API DLLIMPORT
#endif

#endif //CORE_LIBHLCORE_H