#include "shared/Utility.h"

#include "CCVarSystem.h"

#include "CVarUtils.h"

namespace cvar
{
void ColorCVarChanged( cvar::CCVar& cvar, const char* pszOldValue, float flOldValue )
{
	const int iNewValue = clamp( cvar.GetInt(), 0, 255 );

	if( iNewValue != cvar.GetInt() )
	{
		cvar.SetInt( iNewValue );
	}
}

bool GetColorCVars( const char* const pszCVar, CCVar** ppR, CCVar** ppG, CCVar** ppB )
{
	assert( pszCVar );

	char szBuffer[ MAX_BUFFER_LENGTH ];

	if( ppR )
	{
		snprintf( szBuffer, sizeof( szBuffer ), "%s_r", pszCVar );
		*ppR = static_cast<cvar::CCVar*>( cvar::cvars().FindCommand( szBuffer ) );
	}

	if( ppG )
	{
		snprintf( szBuffer, sizeof( szBuffer ), "%s_g", pszCVar );
		*ppG = static_cast<cvar::CCVar*>( cvar::cvars().FindCommand( szBuffer ) );
	}

	if( ppB )
	{
		snprintf( szBuffer, sizeof( szBuffer ), "%s_b", pszCVar );
		*ppB = static_cast<cvar::CCVar*>( cvar::cvars().FindCommand( szBuffer ) );
	}

	return 
		( !ppR || *ppR ) &&
		( !ppG || *ppG ) &&
		( !ppB || *ppB );
}
}