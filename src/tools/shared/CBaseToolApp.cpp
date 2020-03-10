#include <filesystem>

#include "core/LibHLCore.h"
#include "core/shared/Logging.h"
#include "core/shared/Utility.h"

#include "utility/PlatUtils.h"

#include "cvar/CVar.h"
#include "cvar/CCVarSystem.h"

#include "filesystem/CFileSystem.h"
#include "filesystem/IFileSystem.h"
#include "soundsystem/CSoundSystem.h"
#include "soundsystem/ISoundSystem.h"

#include "engine/shared/renderer/IRendererLibrary.h"
#include "engine/shared/renderer/IRenderContext.h"
#include "engine/shared/renderer/studiomodel/IStudioModelRenderer.h"

#include "CBaseToolApp.h"

studiomdl::IStudioModelRenderer* g_pStudioMdlRenderer = nullptr;
soundsystem::ISoundSystem* g_pSoundSystem = nullptr;

//TODO: remove
renderer::IRenderContext* g_pRenderContext = nullptr;

namespace tools
{
bool CBaseToolApp::StartupApp()
{
	//No log file provided, initialize to executable filename.
	if( m_szLogFilename.empty() )
	{
		bool bSuccess;
		auto szExePath = plat::GetExeFileName( &bSuccess );

		if( bSuccess )
		{
			std::filesystem::path path( szExePath );

			m_szLogFilename = path.stem().string();
		}

		if( m_szLogFilename.empty() )
		{
			Warning( "CBaseToolApp::StartupApp: Couldn't get executable name, defaulting log filename to \"tool\"\n" );
			m_szLogFilename = "tool";
		}
	}

	const std::string szLogFilename = m_szLogFilename + ".log";

	//Overwrite previous session log.
	logging().OpenLogFile( szLogFilename.c_str(), false );

	UTIL_InitRandom();

	return true;
}

bool CBaseToolApp::LoadAppLibraries()
{
	if( !LoadLibraries( "Renderer" ) )
		return false;

	return true;
}

bool CBaseToolApp::Connect( const CreateInterfaceFn* pFactories, const size_t uiNumFactories )
{
	if( !LoadAndCheckInterfaces( pFactories, uiNumFactories, 
							IFace( IRENDERERLIBRARY_NAME, m_pRendererLib, "Render Library" ),
							IFace( IRENDERCONTEXT_NAME, g_pRenderContext, "Render Context" ),
							IFace( ISTUDIOMODELRENDERER_NAME, g_pStudioMdlRenderer, "StudioModel Renderer" ) ) )
	{
		return false;
	}

	//TODO: this is temporary until the modular design gets refactored out
	//TODO: need to delete these at some point
	m_pFileSystem = new filesystem::CFileSystem();
	g_pCVar = new cvar::CCVarSystem();
	g_pSoundSystem = m_pSoundSystem = new soundsystem::CSoundSystem();

	if( !g_pCVar->Initialize() )
	{
		FatalError( "Failed to initialize CVar system!\n" );
		return false;
	}

	//Connect Core lib cvars first.
	ConnectCoreCVars( g_pCVar );

	if( !m_pFileSystem->Initialize() )
	{
		FatalError( "Failed to initialize file system!\n" );
		return false;
	}

	if( !InitOpenGL() )
	{
		return false;
	}

	if( !m_pRendererLib->Connect( pFactories, uiNumFactories ) )
	{
		FatalError( "Failed to connect renderer!\n" );
		return false;
	}

	if( !g_pStudioMdlRenderer->Initialize() )
	{
		FatalError( "Failed to initialize StudioModel renderer!\n" );
		return false;
	}

	if( !m_pSoundSystem->Connect( pFactories, uiNumFactories ) )
	{
		FatalError( "Failed to connect sound system!\n" );
		return false;
	}

	if( !m_pSoundSystem->Initialize() )
	{
		FatalError( "Failed to initialize sound system!\n" );
		return false;
	}

	return true;
}

void CBaseToolApp::ShutdownApp()
{
	if( m_pSoundSystem )
	{
		m_pSoundSystem->Shutdown();
		m_pSoundSystem->Disconnect();
		m_pSoundSystem = nullptr;
	}

	if( g_pStudioMdlRenderer )
	{
		g_pStudioMdlRenderer->Shutdown();
		g_pStudioMdlRenderer = nullptr;
	}

	if( g_pRenderContext )
	{
		g_pRenderContext = nullptr;
	}

	if( m_pRendererLib )
	{
		m_pRendererLib->Disconnect();
		m_pRendererLib = nullptr;
	}

	ShutdownOpenGL();

	if( m_pFileSystem )
	{
		m_pFileSystem->Shutdown();
		m_pFileSystem = nullptr;
	}

	if( g_pCVar )
	{
		g_pCVar->Shutdown();
		g_pCVar = nullptr;
	}

	//Don't close the log file just yet. It'll be closed when the program is unloaded, so anything that happens between now and then should be logged.
}
}