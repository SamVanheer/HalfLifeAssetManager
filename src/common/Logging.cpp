#include <cassert>
#include <cstdarg>
#include <cstdio>

#include "Logging.h"

namespace
{
class CNullLogListener final : public ILogListener
{
	void LogMessage( const LogType type, const char* const pszMessage ) override final
	{
		//Nothing.
	}
};

static CNullLogListener g_NullLogListener;

static CLogging g_Logging;
}

ILogListener* GetNullLogListener()
{
	return &g_NullLogListener;
}

CLogging& logging()
{
	return g_Logging;
}

CLogging::CLogging()
{
	SetLogListener( GetDefaultLogListener() );
}

CLogging::~CLogging()
{
}

void CLogging::SetLogListener( ILogListener* pListener )
{
	if( pListener )
	{
		m_pListener = pListener;
	}
	else
	{
		m_pListener = GetDefaultLogListener();
	}
}

/*
*	Common logging interface implementation.
*/
namespace
{
//Don't trigger recursive logging.
static bool g_bInLog = false;

static void Log( const char* const pszFormat, va_list list, const LogType type )
{
	char szBuffer[ 8192 ];

	//Don't rely on wxString's formatting here, it breaks for some reason.
	const int iRet = vsnprintf( szBuffer, sizeof( szBuffer ), pszFormat, list );

	if( iRet < 0 || static_cast<size_t>( iRet ) >= sizeof( szBuffer ) )
	{
		snprintf( szBuffer, sizeof( szBuffer ), "Log buffer too small for '%s'\n", pszFormat );
	}

	logging().GetLogListener()->LogMessage( type, szBuffer );
}
}

void Message( const char* const pszFormat, ... )
{
	if( g_bInLog )
		return;

	g_bInLog = true;

	assert( pszFormat != nullptr && *pszFormat );

	va_list list;

	va_start( list, pszFormat );

	Log( pszFormat, list, LogType::MESSAGE );

	va_end( list );

	g_bInLog = false;
}

void Warning( const char* const pszFormat, ... )
{
	if( g_bInLog )
		return;

	g_bInLog = true;

	assert( pszFormat != nullptr && *pszFormat );

	va_list list;

	va_start( list, pszFormat );

	Log( pszFormat, list, LogType::WARNING );

	va_end( list );

	g_bInLog = false;
}

void Error( const char* const pszFormat, ... )
{
	if( g_bInLog )
		return;

	g_bInLog = true;

	assert( pszFormat != nullptr && *pszFormat );

	va_list list;

	va_start( list, pszFormat );

	Log( pszFormat, list, LogType::ERROR );

	va_end( list );

	g_bInLog = false;
}