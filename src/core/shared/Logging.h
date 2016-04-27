#ifndef COMMON_LOGGING_H
#define COMMON_LOGGING_H

#include <cstdarg>
#include <cstdio>

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
const char* GetLogTypePrefix( const LogType type );

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
ILogListener* GetNullLogListener();

/**
*	Gets the listener that outputs to stdout.
*/
ILogListener* GetStdOutLogListener();

/**
*	Gets the default log listener. If there is a way to log anything, this will provide a means to do so.
*	Must be installed by the application itself.
*/
ILogListener* GetDefaultLogListener();

/**
*	Sets the default log listener.
*/
void SetDefaultLogListener( ILogListener* pListener );

/**
*	This class manages logging state.
*/
class CLogging final
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
	*	@param pszFormat Format string.
	*	@param list Varargs list.
	*/
	void VLog( const LogType type, const char* const pszFormat, va_list list );

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
CLogging& logging();

/**
*	The implementations for logging features are UI dependent.
*/

/**
*	Logs a message.
*/
void Message( const char* const pszFormat, ... );

/**
*	Logs a warning.
*/
void Warning( const char* const pszFormat, ... );

/**
*	Logs an error.
*/
void Error( const char* const pszFormat, ... );

#endif //COMMON_LOGGING_H
