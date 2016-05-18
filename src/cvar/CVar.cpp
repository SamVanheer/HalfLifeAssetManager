#include <cassert>

#include "CVar.h"

cvar::ICVarSystem* g_pCVar = nullptr;

namespace cvar
{
void ConnectCVars()
{
	assert( g_pCVar );

	for( auto pCVar = cvar::CBaseConCommand::GetHead(); pCVar; pCVar = pCVar->GetNext() )
	{
		g_pCVar->AddCommand( pCVar );
	}
}
}