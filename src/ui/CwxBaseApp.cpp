#include "filesystem/CFileSystem.h"
#include "soundsystem/CSoundSystem.h"

#include "CwxOpenGL.h"

#include "ui/shared/CMessagesWindow.h"

#include "common/Utility.h"

#include "CwxBaseApp.h"

CwxBaseApp::CwxBaseApp()
{
}

CwxBaseApp::~CwxBaseApp()
{
}

void CwxBaseApp::OnTimer( CTimer& timer )
{
	//TODO: move this
	if( soundsystem::CSoundSystem::InstanceExists() )
		soundSystem().RunFrame();
}

void CwxBaseApp::OnWindowClose( wxFrame* pWindow, wxCloseEvent& event )
{
	if( pWindow == m_pMessagesWindow )
	{
		if( !event.CanVeto() )
		{
			MessagesWindowClosed();
		}
	}
}

void CwxBaseApp::ExitApp( const bool bMainWndClosed )
{
	//Don't call multiple times.
	if( m_bExiting )
		return;

	m_bExiting = true;

	m_pTimer->SetListener( nullptr );
	
	//Close messages window if needed.
	UseMessagesWindow( false );
}

void CwxBaseApp::UseMessagesWindow( const bool bUse )
{
	//Don't allow creation during exit.
	if( bUse && m_bExiting )
		return;

	//No change
	if( bUse == ( m_pMessagesWindow != nullptr ) )
		return;

	if( bUse )
	{
		m_pMessagesWindow = new CMessagesWindow( m_uiMaxMessagesCount, this );

		logging().SetLogListener( m_pMessagesWindow );
	}
	else
	{
		m_pMessagesWindow->Close( true );

		//Don't call MessagesWindowClosed; Close calls this.
	}
}

void CwxBaseApp::ShowMessagesWindow( const bool bShow )
{
	if( !m_pMessagesWindow )
		return;

	m_pMessagesWindow->Show( bShow );
}

void CwxBaseApp::MessagesWindowClosed()
{
	m_pMessagesWindow = nullptr;

	//TODO: track and reset listener so it restores the previous one
	logging().SetLogListener( nullptr );
}

void CwxBaseApp::SetMaxMessagesCount( const size_t uiMaxMessagesCount )
{
	m_uiMaxMessagesCount = uiMaxMessagesCount;
}

bool CwxBaseApp::InitApp( InitFlags_t initFlags, const wxString& szDisplayName )
{
	UTIL_InitRandom();

	//The sound system requires the use of the file system.
	if( initFlags & INIT_SOUNDSYSTEM )
		initFlags |= INIT_FILESYSTEM;

	if( initFlags & INIT_FILESYSTEM )
	{
		filesystem::CFileSystem::CreateInstance();

		if( !fileSystem().Initialize() )
		{
			wxMessageBox( "Failed to initialize file system", wxMessageBoxCaptionStr, wxOK | wxCENTRE | wxICON_ERROR );
			return false;
		}
	}

	if( !szDisplayName.IsEmpty() )
		SetAppDisplayName( szDisplayName );

	//TODO: make customizable based on app
	if( initFlags & INIT_OPENGL )
	{
		//Set up OpenGL parameters.
		CwxOpenGL::CreateInstance();

		wxGLAttributes canvasAttributes;

		canvasAttributes
			.PlatformDefaults()
			.Stencil( 8 )
			.EndList();

		wxGLContextAttrs contextAttributes;

		contextAttributes.PlatformDefaults()
			.MajorVersion( 3 )
			.MinorVersion( 0 )
			.EndList();

		if( !wxOpenGL().Initialize( canvasAttributes, &contextAttributes ) )
			return false;
	}

	if( initFlags & INIT_IMAGEHANDLERS )
	{
		wxInitAllImageHandlers();
	}

	if( initFlags & INIT_SOUNDSYSTEM )
	{
		soundsystem::CSoundSystem::CreateInstance();

		if( !soundSystem().Initialize() )
		{
			wxMessageBox( "Failed to initialize sound system", wxMessageBoxCaptionStr, wxOK | wxCENTRE | wxICON_ERROR );
			return false;
		}
	}

	m_pTimer = new CTimer( this );

	return true;
}

void CwxBaseApp::ShutdownApp()
{
	if( m_pTimer )
	{
		delete m_pTimer;
		m_pTimer = nullptr;
	}

	if( soundsystem::CSoundSystem::InstanceExists() )
	{
		soundSystem().Shutdown();
		soundsystem::CSoundSystem::DestroyInstance();
	}

	if( filesystem::CFileSystem::InstanceExists() )
	{
		fileSystem().Shutdown();
		filesystem::CFileSystem::DestroyInstance();
	}

	if( CwxOpenGL::InstanceExists() )
	{
		wxOpenGL().Shutdown();

		CwxOpenGL::DestroyInstance();
	}
}

void CwxBaseApp::StartTimer()
{
	//60 FPS
	m_pTimer->Start( ( 1 / 60.0 ) * 1000 );
}