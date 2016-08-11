#ifndef APP_CAPPSYSTEM_H
#define APP_CAPPSYSTEM_H

#include <type_traits>
#include <vector>

#include "lib/CLibrary.h"
#include "lib/LibInterface.h"

#undef LoadLibrary

namespace app
{
/**
*	Current app state.
*/
enum class AppState
{
	CONSTRUCTING = 0,
	STARTING_UP,
	LOADING_LIBS,
	LOADING_IFACES,
	RUNNING,
	SHUTTING_DOWN,
	SHUT_DOWN
};

/**
*	Base class used to automate the loading of libraries and app initialization. This represents the entire application.
*/
class CAppSystem
{
private:
	typedef std::vector<CLibrary> Libraries_t;

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
	*	Lets the app load any libraries it may need.
	*	@return true on success, false otherwise.
	*/
	virtual bool LoadAppLibraries() { return true; }

	/**
	*	Lets the app get any interfaces it needs.
	*	@param pFactories List of factories.
	*	@param uiNumFactories Number of factories.
	*	@return true on success, false otherwise.
	*/
	virtual bool Connect( const CreateInterfaceFn* pFactories, const size_t uiNumFactories ) { return true; }

	/**
	*	Lets the app initialize itself.
	*	@return true on success, false otherwise.
	*/
	virtual bool Initialize() { return true; }

	/**
	*	Run the app.
	*	@param iArgc Argument count.
	*	@param pszArgV Argument vector.
	*	@return true on success, false otherwise.
	*/
	virtual bool RunApp( int iArgc, wchar_t* pszArgV[] ) = 0;

	/**
	*	Lets the app clean up on shutdown.
	*	This is called even if app startup failed.
	*/
	virtual void ShutdownApp() {}

protected:
	/**
	*	Gets the list of libraries.
	*	@return The list of libraries.
	*/
	const Libraries_t& GetLibraries() const { return m_Libraries; }

	/**
	*	Gets a library by name.
	*	@param pszName Name of the library, including path and extension.
	*	@return Library, or null if no such library was loaded.
	*/
	const CLibrary* GetLibraryByName( const char* const pszName ) const;

	/**
	*	Checks whether the given library is loaded.
	*	@param pszName Name of the library to check. Includes path and extension.
	*	@return true if the library is loaded, false otherwise.
	*/
	bool IsLibraryLoaded( const char* const pszName ) const;

	/**
	*	Loads a single library.
	*	@param pszName Library name. Includes path and extension.
	*	@return true on success, false otherwise.
	*/
	bool LoadLibrary( const char* const pszName );

	/**
	*	Loads a number of libraries.
	*	@param pszLibrary Name of the library to load.
	*	@param libraries Libraries to load next.
	*	@return true if all libraries were loaded successfully; false otherwise.
	*/
	template<typename... LIBRARIES>
	bool LoadLibraries( const char* pszLibrary, LIBRARIES... libraries )
	{
		if( !LoadLibrary( pszLibrary ) )
			return false;

		return LoadLibraries( libraries... );
	}

	/**
	*	Overload for LoadLibraries for single argument case.
	*	@see LoadLibraries( const char* pszLibrary, LIBRARIES... libraries )
	*/
	bool LoadLibraries( const char* const pszLibrary )
	{
		return LoadLibrary( pszLibrary );
	}

	/**
	*	Interface descriptor for interface loading.
	*/
	template<typename T>
	struct Interface_t
	{
		const char* const pszInterfaceName;
		T*& pInterface;
		const char* const pszDescription;
		const bool bOptional;
	};

	/**
	*	Helper function to load interfaces. Creates an interface descriptor.
	*	@param pszInterfaceName Name of the interface to load.
	*	@param pInterface Pointer that should receive the interface.
	*	@param pszDescription Optional description to display if the interface failed to load. If null, uses pszInterfaceName.
	*	@param bOptional If true, failure to load the interface does not trigger an error.
	*/
	template<typename T>
	static Interface_t<T> IFace( const char* const pszInterfaceName, T*& pInterface, const char* const pszDescription = nullptr, const bool bOptional = false )
	{
		return Interface_t<T>{ pszInterfaceName, pInterface, pszDescription ? pszDescription : pszInterfaceName, bOptional };
	}

	/**
	*	Loads a number of interfaces.
	*	@param factory Factory to check for interfaces.
	*	@param iface Interface to load.
	*	@param interfaces Interfaces to load next.
	*/
	template<typename INTERFACE, typename... INTERFACES>
	void LoadInterfaces( CreateInterfaceFn factory, INTERFACE iface, INTERFACES... interfaces ) const
	{
		LoadInterfaces( factory, iface );

		LoadInterfaces( factory, interfaces... );
	}

	/**
	*	Overload for LoadInterfaces for single argument case.
	*	@see LoadInterfaces( CreateInterfaceFn factory, INTERFACE iface, INTERFACES... interfaces )
	*/
	template<typename INTERFACE>
	void LoadInterfaces( CreateInterfaceFn factory, INTERFACE iface ) const
	{
		if( iface.pInterface )
			return;

		iface.pInterface = static_cast<std::remove_reference<decltype( iface.pInterface )>::type>( factory( iface.pszInterfaceName, nullptr ) );
	}

	/**
	*	Checks an interface to see if it was initialized successfully.
	*	@param iface Interface descriptor.
	*	@return true if the interface was initialized successfully, false otherwise.
	*/
	template<typename INTERFACE>
	bool CheckInterfaces( INTERFACE iface ) const
	{
		if( !iface.pInterface )
		{
			if( !iface.bOptional )
			{
				Error( "Failed to load interface \"%s\"!\n", iface.pszDescription );
				return false;
			}
			//TODO: consider outputting info in dev mode indicating failure to load optional interfaces.
		}

		return true;
	}

	/**
	*	Overload for CheckInterfaces for multiple interfaces.
	*	@param iface Interface descriptor.
	*	@param interfaces Descriptors to check next.
	*	@see CheckInterfaces( INTERFACE iface )
	*/
	template<typename INTERFACE, typename... INTERFACES>
	bool CheckInterfaces( INTERFACE iface, INTERFACES... interfaces ) const
	{
		if( !CheckInterfaces( iface ) )
			return false;

		return CheckInterfaces( interfaces... );
	}

	/**
	*	Loads and checks a list of interfaces.
	*	@param pFactories List of factories.
	*	@param uiNumFactories Number of factories in pFactories.
	*	@param interfaces Descriptors to check.
	*	@return true if all interfaces loaded successfully, false otherwise.
	*/
	template<typename... INTERFACES>
	bool LoadAndCheckInterfaces( const CreateInterfaceFn* pFactories, const size_t uiNumFactories, INTERFACES&&... interfaces ) const
	{
		for( size_t uiIndex = 0; uiIndex < uiNumFactories; ++uiIndex )
		{
			LoadInterfaces( pFactories[ uiIndex ], std::forward<INTERFACES>( interfaces )... );
		}

		return CheckInterfaces( interfaces... );
	}

private:
	AppState m_State = AppState::CONSTRUCTING;

	Libraries_t m_Libraries;

private:
	CAppSystem( const CAppSystem& ) = delete;
	CAppSystem& operator=( const CAppSystem& ) = delete;
};
}

#endif //APP_CAPPSYSTEM_H