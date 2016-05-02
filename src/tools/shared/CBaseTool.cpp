#include <wx/stdpaths.h>
#include <wx/filename.h>

#include "cvar/CCVarSystem.h"
#include "filesystem/IFileSystem.h"
#include "soundsystem/ISoundSystem.h"

#include "ui/wx/CwxOpenGL.h"

#include "ui/wx/shared/CMessagesWindow.h"

#include "shared/Const.h"
#include "shared/CWorldTime.h"
#include "shared/Utility.h"

#include "settings/CBaseSettings.h"

#include "game/entity/CEntityManager.h"

#include "shared/studiomodel/CStudioModelRenderer.h"

#include "CBaseTool.h"

soundsystem::ISoundSystem* g_pSoundSystem = nullptr;

namespace tools
{
CBaseTool::CBaseTool( const wxString szDisplayName, const wxIcon& toolIcon )
	: m_szDisplayName( szDisplayName )
	, m_ToolIcon( toolIcon )
{
	wxASSERT_MSG( !szDisplayName.IsEmpty(), "Tool Display Name may not be empty!" );

	SetLogFileName( szDisplayName );
}

CBaseTool::~CBaseTool()
{
	delete m_pSettings;
}

void CBaseTool::OnTimer( CTimer& timer )
{
	ToolRunFrame();
}

void CBaseTool::OnWindowClose( wxFrame* pWindow, wxCloseEvent& event )
{
	if( pWindow == m_pMessagesWindow )
	{
		if( !event.CanVeto() )
		{
			MessagesWindowClosed();
		}
	}
}

void CBaseTool::FPSChanged( const double flOldFPS, const double flNewFPS )
{
	//Don't restart the timer unless it's already running (we're changing the FPS, not starting the timer)
	if( IsTimerRunning() )
	{
		StartTimer( flNewFPS );
	}
}

bool CBaseTool::Initialize()
{
	//Configure the working directory to be the exe directory.
	{
		wxStandardPaths& paths = wxStandardPaths::Get();

		const wxString szGetExePath = paths.GetExecutablePath();

		wxFileName szFile( szGetExePath );

		const wxString szWorkingDir = szFile.GetPath();

		wxSetWorkingDirectory( szWorkingDir );
	}

	const wxString szLogFileName = GetLogFileName() + ".log";

	//Overwrite previous session log.
	logging().OpenLogFile( szLogFileName.c_str(), false );

	UTIL_InitRandom();

	//TODO: these message boxes can be put into an UTIL_FatalError function or something
	if( !cvar::cvars().Initialize() )
	{
		wxMessageBox( "Failed to initialize cvar system", wxMessageBoxCaptionStr, wxOK | wxCENTRE | wxICON_ERROR );
		return false;
	}

	//This can probably be automated.
	if( !m_FileSystemLib.Load( "LibFileSystem.dll" ) )
	{
		wxMessageBox( "Failed to load file system library", wxMessageBoxCaptionStr, wxOK | wxCENTRE | wxICON_ERROR );
		return false;
	}

	const CreateInterfaceFn fileSystemFactory = static_cast<CreateInterfaceFn>( m_FileSystemLib.GetFunctionAddress( CREATEINTERFACE_NAME ) );

	if( !fileSystemFactory )
	{
		wxMessageBox( "Failed to get file system factory", wxMessageBoxCaptionStr, wxOK | wxCENTRE | wxICON_ERROR );
		return false;
	}

	m_pFileSystem = static_cast<filesystem::IFileSystem*>( fileSystemFactory( IFILESYSTEM_NAME, nullptr ) );

	if( !m_pFileSystem->Initialize() )
	{
		wxMessageBox( "Failed to initialize file system", wxMessageBoxCaptionStr, wxOK | wxCENTRE | wxICON_ERROR );
		return false;
	}

	//Set up OpenGL parameters.
	CwxOpenGL::CreateInstance();

	wxGLAttributes canvasAttributes;

	GetGLCanvasAttributes( canvasAttributes );

	wxGLContextAttrs contextAttributes;

	GetGLContextAttributes( contextAttributes );

	if( !wxOpenGL().Initialize( canvasAttributes, &contextAttributes ) )
	{
		wxMessageBox( "Failed to initialize file system", wxMessageBoxCaptionStr, wxOK | wxCENTRE | wxICON_ERROR );
		return false;
	}

	if( !studiomodel::renderer().Initialize() )
	{
		wxMessageBox( "Failed to initialize StudioModel renderer", wxMessageBoxCaptionStr, wxOK | wxCENTRE | wxICON_ERROR );
		return false;
	}

	wxInitAllImageHandlers();

	if( !m_SoundSystemLib.Load( "LibSoundSystem.dll" ) )
	{
		wxMessageBox( "Failed to load sound system library", wxMessageBoxCaptionStr, wxOK | wxCENTRE | wxICON_ERROR );
		return false;
	}

	const CreateInterfaceFn soundSystemFactory = static_cast<CreateInterfaceFn>( m_SoundSystemLib.GetFunctionAddress( CREATEINTERFACE_NAME ) );

	if( !soundSystemFactory )
	{
		wxMessageBox( "Failed to get sound system factory", wxMessageBoxCaptionStr, wxOK | wxCENTRE | wxICON_ERROR );
		return false;
	}

	g_pSoundSystem = m_pSoundSystem = static_cast<soundsystem::ISoundSystem*>( soundSystemFactory( ISOUNDSYSTEM_NAME, nullptr ) );

	if( !m_pSoundSystem->Connect( &CreateInterface, fileSystemFactory ) )
	{
		wxMessageBox( "Failed to connect sound system", wxMessageBoxCaptionStr, wxOK | wxCENTRE | wxICON_ERROR );
		return false;
	}

	if( !m_pSoundSystem->Initialize() )
	{
		wxMessageBox( "Failed to initialize sound system", wxMessageBoxCaptionStr, wxOK | wxCENTRE | wxICON_ERROR );
		return false;
	}

	if( !EntityManager().Initialize() )
	{
		wxMessageBox( "Failed to initialize entity manager", wxMessageBoxCaptionStr, wxOK | wxCENTRE | wxICON_ERROR );
		return false;
	}

	m_pSettings = CreateSettings();

	if( !m_pSettings )
	{
		wxMessageBox( "Failed to create settings", wxMessageBoxCaptionStr, wxOK | wxCENTRE | wxICON_ERROR );
		return false;
	}

	//TODO: move this
	if( !EntityManager().OnMapBegin() )
	{
		wxMessageBox( "Failed to initialize start map", wxMessageBoxCaptionStr, wxOK | wxCENTRE | wxICON_ERROR );
		return false;
	}

	m_pTimer = new CTimer( this );

	if( !PostInitialize() )
	{
		wxMessageBox( "Failed to post initialize tool", wxMessageBoxCaptionStr, wxOK | wxCENTRE | wxICON_ERROR );
		return false;
	}

	return true;
}

void CBaseTool::GetGLCanvasAttributes( wxGLAttributes& attrs )
{
	attrs
		.PlatformDefaults()
		.Defaults()
		.Stencil( 8 )
		.DoubleBuffer()
		.EndList();
}

void CBaseTool::GetGLContextAttributes( wxGLContextAttrs& attrs )
{
	//The default settings for OpenGL allow for Windows XP support. OpenGL 2.1 is typically supported by XP era hardware, making it the best choice.
	//2.1 supports GLSL 120, which has some features that are nice to have in shaders.
	attrs
		.PlatformDefaults()
		.MajorVersion( 2 )
		.MinorVersion( 1 )
		.EndList();
}

void CBaseTool::Shutdown()
{
	PreShutdown();

	if( m_pTimer )
	{
		delete m_pTimer;
		m_pTimer = nullptr;
	}

	if( EntityManager().IsMapRunning() )
	{
		EntityManager().OnMapEnd();
	}

	EntityManager().Shutdown();

	if( m_pSoundSystem )
	{
		m_pSoundSystem->Shutdown();

		m_pSoundSystem->Disconnect();

		m_pSoundSystem = nullptr;
		g_pSoundSystem = nullptr;
	}

	m_SoundSystemLib.Free();

	if( CwxOpenGL::InstanceExists() )
	{
		studiomodel::renderer().Shutdown();

		wxOpenGL().Shutdown();

		CwxOpenGL::DestroyInstance();
	}

	if( m_pFileSystem )
	{
		m_pFileSystem->Shutdown();
		m_pFileSystem = nullptr;
	}

	m_FileSystemLib.Free();

	cvar::cvars().Shutdown();

	//Don't close the log file just yet. It'll be closed when the program is unloaded, so anything that happens between now and then should be logged.
}

void CBaseTool::ToolRunFrame()
{
	const double flCurTime = GetCurrentTime();

	double flFrameTime = flCurTime - WorldTime.GetPreviousRealTime();

	WorldTime.SetRealTime( flCurTime );

	if( flFrameTime > 1.0 )
		flFrameTime = 0.1;

	//Don't use this when using wxTimer, since it lowers the FPS by a fair amount.
	/*
	if( flFrameTime < ( 1.0 / GetSettings()->GetFPS() ) )
		return;
		*/

	WorldTime.SetPreviousTime( WorldTime.GetCurrentTime() );
	WorldTime.SetCurrentTime( WorldTime.GetCurrentTime() + flFrameTime );
	WorldTime.SetFrameTime( flFrameTime );
	WorldTime.SetPreviousRealTime( WorldTime.GetRealTime() );

	EntityManager().RunFrame();

	cvar::cvars().RunFrame();

	studiomodel::renderer().RunFrame();

	m_pSoundSystem->RunFrame();

	RunFrame();
}

void CBaseTool::Exit( const bool bMainWndClosed )
{
	//Don't call multiple times.
	if( m_bExiting )
		return;

	m_bExiting = true;

	m_pTimer->SetListener( nullptr );

	//Close messages window if needed.
	UseMessagesWindow( false );

	//Don't let any log message boxes pop up during shutdown.
	logging().SetLogListener( GetNullLogListener() );

	OnExit( bMainWndClosed );
}

const wxString& CBaseTool::GetLogFileName() const
{
	return !m_szLogFileName.IsEmpty() ? m_szLogFileName : m_szDisplayName;
}

void CBaseTool::SetLogFileName( const wxString& szFileName )
{
	if( szFileName.IsEmpty() )
		m_szLogFileName = m_szDisplayName;
	else
		m_szLogFileName = szFileName;
}

void CBaseTool::UseMessagesWindow( const bool bUse )
{
	//Don't allow creation during exit.
	if( bUse && m_bExiting )
		return;

	//No change
	if( bUse == ( m_pMessagesWindow != nullptr ) )
		return;

	if( bUse )
	{
		m_pMessagesWindow = new ui::CMessagesWindow( m_uiMaxMessagesCount, this );

		m_pMessagesWindow->SetIcon( GetToolIcon() );

		logging().SetLogListener( m_pMessagesWindow );
	}
	else
	{
		m_pMessagesWindow->Close( true );

		//Don't call MessagesWindowClosed; Close calls this.
	}
}

void CBaseTool::ShowMessagesWindow( const bool bShow )
{
	if( !m_pMessagesWindow )
		return;

	m_pMessagesWindow->Show( bShow );
}

void CBaseTool::MessagesWindowClosed()
{
	m_pMessagesWindow = nullptr;

	logging().SetLogListener( nullptr );
}

size_t CBaseTool::GetMaxMessagesCount() const
{
	if( !m_pMessagesWindow )
		return m_uiMaxMessagesCount;

	return m_pMessagesWindow->GetMaxMessagesCount();
}

void CBaseTool::SetMaxMessagesCount( const size_t uiMaxMessagesCount )
{
	m_uiMaxMessagesCount = uiMaxMessagesCount;

	if( m_pMessagesWindow )
	{
		m_pMessagesWindow->SetMaxMessagesCount( uiMaxMessagesCount );
	}
}

bool CBaseTool::IsTimerRunning() const
{
	return m_pTimer && m_pTimer->IsRunning();
}

void CBaseTool::StartTimer( double flFPS )
{
	//The + 3 here is necessary so the FPS isn't clamped at odd values. 60 FPS is actually 40 FPS without this.
	//TODO: figure out how to handle the main event loop ourselves, then run frame updates while idle. Idle events look promising, but don't run when a menu is open.
	flFPS = clamp( flFPS, settings::CBaseSettings::MIN_FPS, settings::CBaseSettings::MAX_FPS ) + 3;

	m_pTimer->Start( 1000 / flFPS );
}
}