#include "shared/Utility.h"

#include "utility/StringUtils.h"

#include "CVar.h"

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
		if( !PrintfSuccess( snprintf( szBuffer, sizeof( szBuffer ), "%s_r", pszCVar ), sizeof( szBuffer ) ) )
			return false;

		*ppR = static_cast<cvar::CCVar*>( g_pCVar->FindCommand( szBuffer ) );
	}

	if( ppG )
	{
		if( !PrintfSuccess( snprintf( szBuffer, sizeof( szBuffer ), "%s_g", pszCVar ), sizeof( szBuffer ) ) )
			return false;

		*ppG = static_cast<cvar::CCVar*>( g_pCVar->FindCommand( szBuffer ) );
	}

	if( ppB )
	{
		if( !PrintfSuccess( snprintf( szBuffer, sizeof( szBuffer ), "%s_b", pszCVar ), sizeof( szBuffer ) ) )
			return false;

		*ppB = static_cast<cvar::CCVar*>( g_pCVar->FindCommand( szBuffer ) );
	}

	return 
		( !ppR || *ppR ) &&
		( !ppG || *ppG ) &&
		( !ppB || *ppB );
}
}