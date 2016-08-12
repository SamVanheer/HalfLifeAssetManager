#include "cvar/CVar.h"

#include "LibHLCore.h"

void ConnectCoreCVars( cvar::ICVarSystem* pCVarSystem )
{
	cvar::ConnectCVars( pCVarSystem );
}