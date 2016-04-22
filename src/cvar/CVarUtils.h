#ifndef CVAR_CVARUTILS_H
#define CVAR_CVARUTILS_H

#include "CVar.h"

namespace cvar
{
/**
*	CVar callback for color cvars.
*/
void ColorCVarChanged( cvar::CCVar& cvar, const char* pszOldValue, float flOldValue );

/**
*	Gets color CVars. Passing null for any component will cause it to skip that cvar.
*	@param pszCVar The base cvar name.
*	@param ppR Red component.
*	@param ppG Green component.
*	@param ppB Blue component.
*	@return true if all requested components were found, false otherwise.
*/
bool GetColorCVars( const char* const pszCVar, CCVar** ppR, CCVar** ppG, CCVar** ppB );
}

/**
*	Defines 3 cvars with names name_<component>
*/
#define DEFINE_COLOR_CVAR( name, iR, iG, iB, pszHelpInfo, args )											\
static cvar::CCVar name##_r( #name "_r", args.FloatValue( iR ).HelpInfo( pszHelpInfo " Red Color" ) );		\
static cvar::CCVar name##_g( #name "_g", args.FloatValue( iG ).HelpInfo( pszHelpInfo " Green Color" ) );	\
static cvar::CCVar name##_b( #name "_b", args.FloatValue( iB ).HelpInfo( pszHelpInfo " Blue Color" ) )

#endif //CVAR_CVARUTILS_H