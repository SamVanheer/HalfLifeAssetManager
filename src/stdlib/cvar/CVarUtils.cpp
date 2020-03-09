#include <string>

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

	if( ppR )
	{
		const auto name{std::string{pszCVar} + "_r"};

		*ppR = static_cast<cvar::CCVar*>( g_pCVar->FindCommand(name.c_str()) );
	}

	if( ppG )
	{
		const auto name{std::string{pszCVar} +"_g"};

		*ppG = static_cast<cvar::CCVar*>( g_pCVar->FindCommand(name.c_str()) );
	}

	if( ppB )
	{
		const auto name{std::string{pszCVar} +"_b"};

		*ppB = static_cast<cvar::CCVar*>( g_pCVar->FindCommand(name.c_str()) );
	}

	return 
		( !ppR || *ppR ) &&
		( !ppG || *ppG ) &&
		( !ppB || *ppB );
}
}