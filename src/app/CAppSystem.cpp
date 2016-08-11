#include <cassert>

#include <filesystem>

#include "core/shared/Logging.h"

#include "core/shared/Platform.h"

#include "utility/PlatUtils.h"

#include "CAppSystem.h"

namespace app
{
bool CAppSystem::Run( int iArgc, wchar_t* pszArgV[] )
{
	const bool bResult = Startup();

	if( bResult )
	{
		m_State = AppState::RUNNING;

		RunApp( iArgc, pszArgV );
	}

	m_State = AppState::SHUTTING_DOWN;

	ShutdownApp();

	Shutdown();

	m_State = AppState::SHUT_DOWN;

	return bResult;
}

bool CAppSystem::Startup()
{
	//Configure the working directory to be the exe directory.
	{
		bool bSuccess;
		auto szExePath = plat::GetExeFileName( &bSuccess );

		if( !bSuccess )
		{
			Error( "CAppSystem::Startup: Failed to get executable filename!\n" );
			return false;
		}

		std::experimental::filesystem::path exePath( szExePath );

		std::error_code canonialError;

		auto exeDir = exePath.parent_path();

		exeDir = std::experimental::filesystem::canonical( exeDir, canonialError );

		if( canonialError )
		{
			Error(
				"CAppSystem::Startup: Failed to canonicalize  \"%s\" with error \"%s\"!\n",
				exeDir.string().c_str(), canonialError.message().c_str() );
			return false;
		}

		std::error_code cwdError;

		std::experimental::filesystem::current_path( exeDir, cwdError );

		if( cwdError )
		{
			Error( 
				"CAppSystem::Startup: Failed to set current working directory \"%s\" with error \"%s\"!\n", 
				exeDir.string().c_str(), cwdError.message().c_str() );
			return false;
		}
	}

	m_State = AppState::STARTING_UP;

	if( !StartupApp() )
	{
		Error( "CAppSystem::Startup: Failed to start up app!\n" );
		return false;
	}

	m_State = AppState::LOADING_LIBS;

	if( !LoadAppLibraries() )
	{
		Error( "CAppSystem::Startup: Failed to load one or more libraries!\n" );
		return false;
	}

	m_State = AppState::LOADING_IFACES;

	//Create the list of functions for ease of use.
	std::vector<CreateInterfaceFn> factories;

	//Include this library's factory.
	factories.reserve( m_Libraries.size() + 1 );

	factories.push_back( &::CreateInterface );

	for( auto& lib : m_Libraries )
	{
		if( auto pFunc = lib.GetFunctionAddress( CREATEINTERFACE_NAME ) )
		{
			factories.push_back( reinterpret_cast<CreateInterfaceFn>( pFunc ) );
		}
	}

	factories.shrink_to_fit();

	if( !Connect( factories.data(), factories.size() ) )
	{
		Error( "CAppSystem::Startup: Failed to connect one or more interfaces!\n" );
		return false;
	}

	return true;
}

void CAppSystem::Shutdown()
{
	for( auto& lib : m_Libraries )
	{
		lib.Free();
	}

	m_Libraries.clear();
	m_Libraries.shrink_to_fit();
}

const CLibrary* CAppSystem::GetLibraryByName( const char* const pszName ) const
{
	assert( pszName );

	for( auto& lib : m_Libraries )
	{
		if( lib.GetName() == pszName )
			return &lib;
	}

	return nullptr;
}

bool CAppSystem::IsLibraryLoaded( const char* const pszName ) const
{
	return GetLibraryByName( pszName ) != nullptr;
}

bool CAppSystem::LoadLibrary( const char* const pszName )
{
	assert( pszName );

	if( IsLibraryLoaded( pszName ) )
		return true;

	CLibrary lib;

	if( !lib.Load( pszName ) )
	{
		Error( "CAppSystem::LoadLibrary: Failed to load library \"%s\"!\n", pszName );
		return false;
	}

	m_Libraries.emplace_back( std::move( lib ) );

	return true;
}
}