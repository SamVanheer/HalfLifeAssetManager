#ifndef COMMON_LOGGING_H
#define COMMON_LOGGING_H

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
*	Gets the default log listener. If there is a way to log anything, this will provide a means to do so.
*	UI dependent.
*/
ILogListener* GetDefaultLogListener();

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
	ILogListener* GetLogListener() { return m_pListener; }

	/**
	*	Sets the current log listener. If pListener is null, the default log listener is used.
	*	@param pListener Listener to set.
	*/
	void SetLogListener( ILogListener* pListener );

private:
	ILogListener* m_pListener = nullptr;

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