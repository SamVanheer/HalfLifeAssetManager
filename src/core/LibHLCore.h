#ifndef CORE_LIBHLCORE_H
#define CORE_LIBHLCORE_H

#include "shared/Platform.h"

namespace cvar
{
class ICVarSystem;
}

#ifdef LIBHLCORE_EXPORTS
#define HLCORE_API DLLEXPORT
#else
#define HLCORE_API DLLIMPORT
#endif

/**
*	Connects core cvars with the given cvar system.
*	This is needed since the core doesn't link up with other libraries at the moment. The calling library should load the cvar system and pass it in.
*	@param pCVarSystem CVar system to connect with.
*/
extern "C" HLCORE_API void ConnectCoreCVars( cvar::ICVarSystem* pCVarSystem );

#endif //CORE_LIBHLCORE_H