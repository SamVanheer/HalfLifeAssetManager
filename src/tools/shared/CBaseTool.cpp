#include "filesystem/CFileSystem.h"
#include "soundsystem/CSoundSystem.h"

#include "ui/CwxOpenGL.h"

#include "ui/shared/CMessagesWindow.h"

#include "common/Utility.h"

#include "CBaseTool.h"

namespace tools
{
CBaseTool::CBaseTool( const InitFlags_t initFlags, const wxString szDisplayName )
	: m_InitFlags( initFlags )
	, m_szDisplayName( szDisplayName )
{
	//The sound system requires the use of the file system.
	if( m_InitFlags & INIT_SOUNDSYSTEM )
		m_InitFlags |= INIT_FILESYSTEM;
}

CBaseTool::~CBaseTool()
{
}

void CBaseTool::OnTimer( CTimer& timer )
{
	if( soundsystem::CSoundSystem::InstanceExists() )
		soundSystem().RunFrame();

	RunFrame();
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

bool CBaseTool::Initialize()
{
	UTIL_InitRandom();

	if( m_InitFlags & INIT_FILESYSTEM )
	{
		filesystem::CFileSystem::CreateInstance();

		if( !fileSystem().Initialize() )
		{
			wxMessageBox( "Failed to initialize file system", wxMessageBoxCaptionStr, wxOK | wxCENTRE | wxICON_ERROR );
			return false;
		}
	}

	if( m_InitFlags & INIT_OPENGL )
	{
		//Set up OpenGL parameters.
		CwxOpenGL::CreateInstance();

		wxGLAttributes canvasAttributes;

		GetGLCanvasAttributes( canvasAttributes );

		wxGLContextAttrs contextAttributes;

		GetGLContextAttributes( contextAttributes );

		if( !wxOpenGL().Initialize( canvasAttributes, &contextAttributes ) )
			return false;
	}

	if( m_InitFlags & INIT_IMAGEHANDLERS )
	{
		wxInitAllImageHandlers();
	}

	if( m_InitFlags & INIT_SOUNDSYSTEM )
	{
		soundsystem::CSoundSystem::CreateInstance();

		if( !soundSystem().Initialize() )
		{
			wxMessageBox( "Failed to initialize sound system", wxMessageBoxCaptionStr, wxOK | wxCENTRE | wxICON_ERROR );
			return false;
		}
	}

	m_pTimer = new CTimer( this );

	if( !PostInitialize() )
		return false;

	return true;
}

void CBaseTool::GetGLCanvasAttributes( wxGLAttributes& attrs )
{
	attrs
		.PlatformDefaults()
		.Stencil( 8 )
		.EndList();
}

void CBaseTool::GetGLContextAttributes( wxGLContextAttrs& attrs )
{
	attrs
		.PlatformDefaults()
		.MajorVersion( 3 )
		.MinorVersion( 0 )
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

void CBaseTool::Exit( const bool bMainWndClosed )
{
	//Don't call multiple times.
	if( m_bExiting )
		return;

	m_bExiting = true;

	m_pTimer->SetListener( nullptr );

	//Close messages window if needed.
	UseMessagesWindow( false );

	OnExit( bMainWndClosed );
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

	//TODO: track and reset listener so it restores the previous one
	logging().SetLogListener( nullptr );
}

void CBaseTool::SetMaxMessagesCount( const size_t uiMaxMessagesCount )
{
	m_uiMaxMessagesCount = uiMaxMessagesCount;
}

void CBaseTool::StartTimer()
{
	//60 FPS
	m_pTimer->Start( ( 1 / 60.0 ) * 1000 );
}
}