#include <cassert>

#include "CVar.h"

cvar::ICVarSystem* g_pCVar = nullptr;

namespace cvar
{
void ConnectCVars( cvar::ICVarSystem* pCVarSystem )
{
	for( auto pCVar = cvar::CBaseConCommand::GetHead(); pCVar; pCVar = pCVar->GetNext() )
	{
		pCVarSystem->AddCommand( pCVar );
	}
}

void ConnectCVars()
{
	assert( g_pCVar );

	ConnectCVars( g_pCVar );
}
}