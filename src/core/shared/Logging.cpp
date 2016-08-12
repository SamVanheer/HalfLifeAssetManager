#include <cassert>
#include <chrono>

#include "utility/StringUtils.h"

#include "cvar/CCVar.h"

#include "Logging.h"

static cvar::CCVar developer( "developer", cvar::CCVarArgsBuilder().HelpInfo( "Developer level for logging" ).FloatValue( 0 ) );

const char* GetLogTypePrefix( const LogType type )
{
	switch( type )
	{
	default:
	case LogType::MESSAGE:	return "";
	case LogType::WARNING:	return "Warning: ";
	case LogType::ERROR:	return "Error: ";
	}
}

const char* DevLevelToString( const DevLevel::DevLevel devLevel )
{
	switch( devLevel )
	{
	default:
	case DevLevel::ALWAYS:		return "ALWAYS";
	case DevLevel::DEV:			return "DEV";
	case DevLevel::VERBOSE:		return "VERBOSE";
	}
}

namespace
{
class CNullLogListener final : public ILogListener
{
public:
	void LogMessage( const LogType type, const char* const pszMessage ) override final
	{
		//Nothing.
	}
};

class CStdOutLogListener final : public ILogListener
{
public:
	void LogMessage( const LogType type, const char* const pszMessage ) override final
	{
		printf( "%s%s", GetLogTypePrefix( type ), pszMessage );
	}
};

static CNullLogListener g_NullLogListener;

static CStdOutLogListener g_StdOutListener;

static ILogListener* m_pDefaultLogListener = &g_NullLogListener;

static CLogging g_Logging;
}

ILogListener* GetNullLogListener()
{
	return &g_NullLogListener;
}

ILogListener* GetStdOutLogListener()
{
	return &g_StdOutListener;
}

ILogListener* GetDefaultLogListener()
{
	return m_pDefaultLogListener;
}

void SetDefaultLogListener( ILogListener* pListener )
{
	assert( pListener );

	m_pDefaultLogListener = pListener;
}

CLogging& logging()
{
	return g_Logging;
}

CLogging::CLogging()
{
}

CLogging::~CLogging()
{
	CloseLogFile();
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

void CLogging::Log( const LogType type, const char* const pszFormat, ... )
{
	assert( pszFormat != nullptr && *pszFormat );

	va_list list;

	va_start( list, pszFormat );

	VLog( type, pszFormat, list );

	va_end( list );
}

void CLogging::Log( const LogType type, const DevLevel::DevLevel devLevel, const char* const pszFormat, ... )
{
	assert( pszFormat != nullptr && *pszFormat );

	va_list list;

	va_start( list, pszFormat );

	VLog( type, devLevel, pszFormat, list );

	va_end( list );
}

void CLogging::VLog( const LogType type, const char* const pszFormat, va_list list )
{
	return VLog( type, DevLevel::ALWAYS, pszFormat, list );
}

void CLogging::VLog( const LogType type, const DevLevel::DevLevel devLevel, const char* const pszFormat, va_list list )
{
	assert( pszFormat != nullptr && *pszFormat );

	if( m_bInLog )
		return;

	if( developer.GetInt() < devLevel )
		return;

	m_bInLog = true;

	char szBuffer[ 8192 ];

	const int iRet = vsnprintf( szBuffer, sizeof( szBuffer ), pszFormat, list );

	if( iRet < 0 || static_cast<size_t>( iRet ) >= sizeof( szBuffer ) )
	{
		if( !PrintfSuccess( snprintf( szBuffer, sizeof( szBuffer ), "Log buffer too small for '%s'\n", pszFormat ), sizeof( szBuffer ) ) )
		{
			strcpy( szBuffer, "Log buffer too small\n" );
		}
	}

	if( IsLogFileOpen() )
	{
		fprintf( m_pLogFile, "%s", szBuffer );
	}

	GetLogListener()->LogMessage( type, szBuffer );

	m_bInLog = false;
}

bool CLogging::OpenLogFile( const char* const pszFilename, const bool bAppend )
{
	assert( pszFilename && *pszFilename );

	CloseLogFile();

	m_pLogFile = fopen( pszFilename, bAppend ? "wa" : "w" );

	if( m_pLogFile )
	{
		auto now = std::chrono::system_clock::now();

		const time_t time = std::chrono::system_clock::to_time_t( now );

		//No newline because ctime's return value contains one.
		fprintf( m_pLogFile, "Log opened on %s", ctime( &time ) );
	}

	return IsLogFileOpen();
}

void CLogging::CloseLogFile()
{
	if( IsLogFileOpen() )
	{
		auto now = std::chrono::system_clock::now();

		const time_t time = std::chrono::system_clock::to_time_t( now );

		//No newline because ctime's return value contains one.
		fprintf( m_pLogFile, "Log closed on %s", ctime( &time ) );

		fclose( m_pLogFile );
		m_pLogFile = nullptr;
	}
}

void Message( const char* const pszFormat, ... )
{
	va_list list;

	va_start( list, pszFormat );

	logging().VLog( LogType::MESSAGE, pszFormat, list );

	va_end( list );
}

void Warning( const char* const pszFormat, ... )
{
	va_list list;

	va_start( list, pszFormat );

	logging().VLog( LogType::WARNING, pszFormat, list );

	va_end( list );
}

void Error( const char* const pszFormat, ... )
{
	va_list list;

	va_start( list, pszFormat );

	logging().VLog( LogType::ERROR, pszFormat, list );

	va_end( list );
}

void DevMsg( const int devLevel, const char* const pszFormat, ... )
{
	va_list list;

	va_start( list, pszFormat );

	logging().VLog( LogType::MESSAGE, static_cast<DevLevel::DevLevel>( devLevel ), pszFormat, list );

	va_end( list );
}

void DevWarning( const int devLevel, const char* const pszFormat, ... )
{
	va_list list;

	va_start( list, pszFormat );

	logging().VLog( LogType::WARNING, static_cast<DevLevel::DevLevel>( devLevel ), pszFormat, list );

	va_end( list );
}

void DevError( const int devLevel, const char* const pszFormat, ... )
{
	va_list list;

	va_start( list, pszFormat );

	logging().VLog( LogType::ERROR, static_cast<DevLevel::DevLevel>( devLevel ), pszFormat, list );

	va_end( list );
}
