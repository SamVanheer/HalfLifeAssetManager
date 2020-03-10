#ifndef APP_CAPPSYSTEM_H
#define APP_CAPPSYSTEM_H

namespace app
{
/**
*	Current app state.
*/
enum class AppState
{
	CONSTRUCTING = 0,
	STARTING_UP,
	INITIALIZING,
	RUNNING,
	SHUTTING_DOWN,
	SHUT_DOWN
};

/**
*	Base class used to automate the loading of libraries and app initialization. This represents the entire application.
*/
class CAppSystem
{
public:
	CAppSystem() = default;
	~CAppSystem() = default;

	/**
	*	@return The current app state.
	*/
	AppState GetState() const { return m_State; }

	/**
	*	Runs the app.
	*	@return Whether the app successfully started or not.
	*/
	bool Run( int iArgc, wchar_t* pszArgV[] );

	/**
	*	Starts the app.
	*/
	bool Start();

	/**
	*	Should be called on shutdown.
	*/
	void OnShutdown();

private:
	/**
	*	Starts up the application.
	*/
	bool Startup();

	/**
	*	Shuts down the app. Ran after app-specific shutdown.
	*/
	void Shutdown();

protected:
	/**
	*	Lets the app run code on startup. This is called after the current working directory has been set.
	*	@return true on success, false otherwise.
	*/
	virtual bool StartupApp() { return true; }

	/**
	*	Lets the app initialize itself.
	*	@return true on success, false otherwise.
	*/
	virtual bool Initialize() { return true; }

	/**
	*	Run the app. Only called if the GUI isn't running the main loop.
	*	@param iArgc Argument count.
	*	@param pszArgV Argument vector.
	*	@return true on success, false otherwise.
	*/
	virtual bool RunApp( int iArgc, wchar_t* pszArgV[] ) { return false; }

	/**
	*	Lets the app clean up on shutdown.
	*	This is called even if app startup failed.
	*/
	virtual void ShutdownApp() {}

private:
	AppState m_State = AppState::CONSTRUCTING;

private:
	CAppSystem( const CAppSystem& ) = delete;
	CAppSystem& operator=( const CAppSystem& ) = delete;
};
}

#endif //APP_CAPPSYSTEM_H