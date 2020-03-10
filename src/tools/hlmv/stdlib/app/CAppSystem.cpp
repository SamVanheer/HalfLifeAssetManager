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
	bool bResult = Start();

	if( bResult )
	{
		RunApp( iArgc, pszArgV );
	}

	OnShutdown();

	return bResult;
}

bool CAppSystem::Start()
{
	bool bResult = Startup();

	if( bResult )
	{
		m_State = AppState::RUNNING;
	}

	return bResult;
}

void CAppSystem::OnShutdown()
{
	m_State = AppState::SHUTTING_DOWN;

	ShutdownApp();

	Shutdown();

	m_State = AppState::SHUT_DOWN;
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

		std::filesystem::path exePath( szExePath );

		std::error_code canonicalError;

		auto exeDir = exePath.parent_path();

		exeDir = std::filesystem::canonical( exeDir, canonicalError );

		if( canonicalError )
		{
			Error(
				"CAppSystem::Startup: Failed to canonicalize  \"%s\" with error \"%s\"!\n",
				exeDir.string().c_str(), canonicalError.message().c_str() );
			return false;
		}

		std::error_code cwdError;

		std::filesystem::current_path( exeDir, cwdError );

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

	m_State = AppState::INITIALIZING;

	if( !Initialize() )
	{
		Error( "CAppSystem::Startup: Failed to initialize app!\n" );
		return false;
	}

	return true;
}

void CAppSystem::Shutdown()
{
}
}