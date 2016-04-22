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
*	Externs the color cvars for name.
*/
#define EXTERN_COLOR_CVAR( name )	\
extern cvar::CCVar name##_r;		\
extern cvar::CCVar name##_g;		\
extern cvar::CCVar name##_b

/**
*	Defines 3 cvars with names name_<component>.
*	@param linkage Variable linkage. Can be empty.
*	@param name Name of the cvars. _r, _g, _b is appended.
*	@param iR Red component value.
*	@param iG Green component value.
*	@param iB Blue component value.
*	@param pszHelpInfo Base help info. Color info is appended.
*	@param args CCVarArgsBuilder instance.
*/
#define DEFINE_COLOR_CVAR( linkage, name, iR, iG, iB, pszHelpInfo, args )									\
linkage cvar::CCVar name##_r( #name "_r", args.FloatValue( iR ).HelpInfo( pszHelpInfo " Red Color" ) );		\
linkage cvar::CCVar name##_g( #name "_g", args.FloatValue( iG ).HelpInfo( pszHelpInfo " Green Color" ) );	\
linkage cvar::CCVar name##_b( #name "_b", args.FloatValue( iB ).HelpInfo( pszHelpInfo " Blue Color" ) )

#endif //CVAR_CVARUTILS_H