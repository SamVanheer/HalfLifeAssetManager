#ifndef CVAR_CVAR_H
#define CVAR_CVAR_H

#include "ConVarConstants.h"

#include "CBaseConCommand.h"
#include "CConCommand.h"
#include "CCVar.h"

#include "ICVarSystem.h"

extern cvar::ICVarSystem* g_pCVar;

namespace cvar
{
/**
*	Connects this library's cvars with the given cvar system.
*/
void ConnectCVars( cvar::ICVarSystem* pCVarSystem );

/**
*	Connects this library's cvars with the cvar system.
*/
void ConnectCVars();
}

#endif //CVAR_CVAR_H
