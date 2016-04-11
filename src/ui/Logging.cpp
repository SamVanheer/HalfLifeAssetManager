#include "wxInclude.h"

#include <cstdarg>
#include <cstring>

#include "common/Logging.h"

//wxWidgets timers will keep going while message boxes are up, so we have to prevent it from causing stack overflows and other problems.
static bool g_bInLog = false;

static void Log( const char* const pszFormat, va_list list, const wxString& szCaption, const long style )
{
	char szBuffer[ 8192 ];

	//Don't rely on wxString's formatting here, it breaks for some reason.
	const int iRet = vsnprintf( szBuffer, sizeof( szBuffer ), pszFormat, list );

	if( iRet < 0 || static_cast<size_t>( iRet ) >= sizeof( szBuffer ) )
	{
		snprintf( szBuffer, sizeof( szBuffer ), "Log buffer too small for '%s'\n", pszFormat );
	}

	wxMessageBox( szBuffer, szCaption, style );
}

void Message( const char* const pszFormat, ... )
{
	if( g_bInLog )
		return;

	g_bInLog = true;

	wxASSERT( pszFormat != nullptr && *pszFormat );

	va_list list;

	va_start( list, pszFormat );

	Log( pszFormat, list, "Message", wxOK | wxCENTRE );

	va_end( list );

	g_bInLog = false;
}

void Warning( const char* const pszFormat, ... )
{
	if( g_bInLog )
		return;

	g_bInLog = true;

	wxASSERT( pszFormat != nullptr && *pszFormat );

	va_list list;

	va_start( list, pszFormat );

	Log( pszFormat, list, "Warning", wxOK | wxCENTRE | wxICON_WARNING );

	va_end( list );

	g_bInLog = false;
}

void Error( const char* const pszFormat, ... )
{
	if( g_bInLog )
		return;

	g_bInLog = true;

	wxASSERT( pszFormat != nullptr && *pszFormat );

	va_list list;

	va_start( list, pszFormat );

	Log( pszFormat, list, "Error", wxOK | wxCENTRE | wxICON_ERROR );

	va_end( list );

	g_bInLog = false;
}