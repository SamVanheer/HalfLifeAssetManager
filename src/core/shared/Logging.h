#ifndef COMMON_LOGGING_H
#define COMMON_LOGGING_H

#include <cstdarg>
#include <cstdio>

#include "core/LibHLCore.h"

#ifdef ERROR
#undef ERROR
#endif

/**
*	Defines the types of messages that can be logged.
*/
enum class LogType
{
	MESSAGE,
	WARNING,
	ERROR
};

/**
*	Gets a prefix to use for the given log type.
*/
extern "C" HLCORE_API const char* GetLogTypePrefix( const LogType type );

namespace DevLevel
{
/**
*	Defines developer log levels.
*	Weakly typed so code can define its own levels more easily.
*/
enum DevLevel
{
	ALWAYS = 0,
	DEV,
	VERBOSE
};
}

extern "C" HLCORE_API const char* DevLevelToString( const DevLevel::DevLevel devLevel );

/**
*	This interface defines a log listener. Classes can implement this to receive log messages.
*/
class ILogListener
{
public:
	virtual ~ILogListener() = 0;

	/**
	*	Log a message of the given type, containing the given message.
	*/
	virtual void LogMessage( const LogType type, const char* const pszMessage ) = 0;
};

inline ILogListener::~ILogListener()
{
}

/**
*	Gets the null log listener. This listener does not log anything.
*/
extern "C" HLCORE_API ILogListener* GetNullLogListener();

/**
*	Gets the listener that outputs to stdout.
*/
extern "C" HLCORE_API ILogListener* GetStdOutLogListener();

/**
*	Gets the default log listener. If there is a way to log anything, this will provide a means to do so.
*	Must be installed by the application itself.
*/
extern "C" HLCORE_API ILogListener* GetDefaultLogListener();

/**
*	Sets the default log listener.
*/
extern "C" HLCORE_API void SetDefaultLogListener( ILogListener* pListener );

/**
*	This class manages logging state.
*/
class HLCORE_API CLogging final
{
public:
	CLogging();
	~CLogging();

	/**
	*	Gets the current log listener.
	*/
	ILogListener* GetLogListener() { return m_pListener ? m_pListener : GetDefaultLogListener(); }

	/**
	*	Sets the current log listener. If pListener is null, the default log listener is used.
	*	@param pListener Listener to set.
	*/
	void SetLogListener( ILogListener* pListener );

	/**
	*	Logs a message.
	*	@param type Message type.
	*	@param pszFormat Format string.
	*	@param ... Varargs parameters.
	*/
	void Log( const LogType type, const char* const pszFormat, ... );

	/**
	*	Logs a message.
	*	@param type Message type.
	*	@param devLevel Developer level.
	*	@param pszFormat Format string.
	*	@param ... Varargs parameters.
	*/
	void Log( const LogType type, const DevLevel::DevLevel devLevel, const char* const pszFormat, ... );

	/**
	*	Logs a message.
	*	@param type Message type.
	*	@param pszFormat Format string.
	*	@param list Varargs list.
	*/
	void VLog( const LogType type, const char* const pszFormat, va_list list );

	/**
	*	Logs a message.
	*	@param type Message type.
	*	@param devLevel Developer level.
	*	@param pszFormat Format string.
	*	@param list Varargs list.
	*/
	void VLog( const LogType type, const DevLevel::DevLevel devLevel, const char* const pszFormat, va_list list );

	bool IsLogFileOpen() const { return m_pLogFile != nullptr; }

	bool OpenLogFile( const char* const pszFilename, const bool bAppend = true );

	void CloseLogFile();

private:
	ILogListener* m_pListener = nullptr;

	//Don't trigger recursive logging.
	bool m_bInLog = false;

	FILE* m_pLogFile = nullptr;

private:
	CLogging( const CLogging& ) = delete;
	CLogging& operator=( const CLogging& ) = delete;
};

/**
*	Gets the log state manager.
*/
extern "C" HLCORE_API CLogging& logging();

/**
*	The implementations for logging features are UI dependent.
*/

/**
*	Logs a message.
*/
extern "C" HLCORE_API void Message( const char* const pszFormat, ... );

/**
*	Logs a warning.
*/
extern "C" HLCORE_API void Warning( const char* const pszFormat, ... );

/**
*	Logs an error.
*/
extern "C" HLCORE_API void Error( const char* const pszFormat, ... );

/**
*	Logs a developer message.
*	@param devLevel Developer level to filter with. If the developer cvar setting is less than this, the message is ignored.
*/
extern "C" HLCORE_API void DevMsg( const int devLevel, const char* const pszFormat, ... );

/**
*	Logs a developer warning.
*	@param devLevel Developer level to filter with. If the developer cvar setting is less than this, the message is ignored.
*/
extern "C" HLCORE_API void DevWarning( const int devLevel, const char* const pszFormat, ... );

/**
*	Logs an developer error.
*	@param devLevel Developer level to filter with. If the developer cvar setting is less than this, the message is ignored.
*/
extern "C" HLCORE_API void DevError( const int devLevel, const char* const pszFormat, ... );

#endif //COMMON_LOGGING_H
