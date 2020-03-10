#include "core/shared/Platform.h"

#include "core/shared/Logging.h"

#include "core/shared/Utility.h"
#include "core/shared/CWorldTime.h"

#include "cvar/CVar.h"

#include "shared/renderer/studiomodel/IStudioModelRenderer.h"
#include "soundsystem/ISoundSystem.h"

#include "ui/wx/wxLogging.h"

#include "ui/common/CMessagesWindow.h"

#include "CBaseWXToolApp.h"

extern studiomdl::IStudioModelRenderer* g_pStudioMdlRenderer;

namespace tools
{
static cvar::CCVar max_fps( 
	"max_fps", 
	cvar::CCVarArgsBuilder()
	.HelpInfo( "The maximum FPS that this program will redraw at" )
	.FloatValue( 60.0f )
	.Flags( cvar::Flag::ARCHIVE )
);

bool CBaseWXToolApp::Initialize()
{
	if( !CBaseToolApp::Initialize() )
		return false;

	wxInitAllImageHandlers();

	return true;
}

bool CBaseWXToolApp::InitOpenGL()
{
	//Set up OpenGL parameters.
	CwxOpenGL::CreateInstance();

	wxGLAttributes canvasAttributes;

	GetGLCanvasAttributes( canvasAttributes );

	wxGLContextAttrs contextAttributes;

	GetGLContextAttributes( contextAttributes );

	if( !wxOpenGL().Initialize( canvasAttributes, &contextAttributes ) )
	{
		wxMessageBox( "Failed to initialize OpenGL", wxMessageBoxCaptionStr, wxOK | wxCENTRE | wxICON_ERROR );
		return false;
	}

	return true;
}

void CBaseWXToolApp::ShutdownOpenGL()
{
	if( CwxOpenGL::InstanceExists() )
	{
		wxOpenGL().Shutdown();

		CwxOpenGL::DestroyInstance();
	}
}

void CBaseWXToolApp::GetGLCanvasAttributes( wxGLAttributes& attrs )
{
	attrs
		.PlatformDefaults()
		.RGBA()
		.Depth( 16 )
		.Stencil( 8 )
		.DoubleBuffer()
		.EndList();
}

void CBaseWXToolApp::GetGLContextAttributes( wxGLContextAttrs& attrs )
{
	//The default settings for OpenGL allow for Windows XP support. OpenGL 2.1 is typically supported by XP era hardware, making it the best choice.
	//2.1 supports GLSL 120, which has some features that are nice to have in shaders.
	attrs
		.PlatformDefaults()
		.MajorVersion( 2 )
		.MinorVersion( 1 )
		.EndList();
}

bool CBaseWXToolApp::OnInit()
{
	if( !wxApp::OnInit() )
		return false;

	//Install the wxWidgets specific default log listener.
	SetDefaultLogListener( GetwxDefaultLogListener() );

	const bool bResult = [ = ]
	{
		if( !Start() )
			return false;

		if( !PreRunApp() )
			return false;

		return true;
	}();

	if( !bResult )
	{
		OnShutdown();

		return false;
	}

	wxApp::Connect( wxEVT_IDLE, wxIdleEventHandler( CBaseWXToolApp::OnIdle ) );

	//Reduce the idle event strain on the system a bit.
	wxIdleEvent::SetMode( wxIDLE_PROCESS_SPECIFIED );

	return true;
}

int CBaseWXToolApp::OnExit()
{
	wxApp::Disconnect( wxEVT_IDLE, wxIdleEventHandler( CBaseWXToolApp::OnIdle ) );

	OnShutdown();

	UseMessagesWindow( false );

	return wxApp::OnExit();
}

void CBaseWXToolApp::Exit( const bool bMainWndClosed )
{
	//Don't call multiple times.
	if( m_bExiting )
		return;

	m_bExiting = true;

	//Close messages window if needed.
	UseMessagesWindow( false );

	//Don't let any log message boxes pop up during shutdown.
	logging().SetLogListener( GetNullLogListener() );

	OnExit( bMainWndClosed );
}

void CBaseWXToolApp::OnWindowClose( wxFrame* pWindow, wxCloseEvent& event )
{
	if( pWindow == m_pMessagesWindow )
	{
		if( !event.CanVeto() )
		{
			MessagesWindowClosed();
		}
	}
}

void CBaseWXToolApp::UseMessagesWindow( const bool bUse )
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

void CBaseWXToolApp::ShowMessagesWindow( const bool bShow )
{
	if( !m_pMessagesWindow )
		return;

	m_pMessagesWindow->Show( bShow );
}

size_t CBaseWXToolApp::GetMaxMessagesCount() const
{
	if( !m_pMessagesWindow )
		return m_uiMaxMessagesCount;

	return m_pMessagesWindow->GetMaxMessagesCount();
}

void CBaseWXToolApp::SetMaxMessagesCount( const size_t uiMaxMessagesCount )
{
	m_uiMaxMessagesCount = uiMaxMessagesCount;

	if( m_pMessagesWindow )
	{
		m_pMessagesWindow->SetMaxMessagesCount( uiMaxMessagesCount );
	}
}

void CBaseWXToolApp::MessagesWindowClosed()
{
	m_pMessagesWindow = nullptr;

	logging().SetLogListener( nullptr );
}

void CBaseWXToolApp::OnIdle( wxIdleEvent& event )
{
	event.RequestMore();

	const double flCurTime = GetCurrentTime();

	double flFrameTime = flCurTime - WorldTime.GetPreviousRealTime();

	WorldTime.SetRealTime( flCurTime );

	if( flFrameTime > 1.0 )
		flFrameTime = 0.1;

	//Don't use this when using wxTimer, since it lowers the FPS by a fair amount.
	if( flFrameTime < ( 1.0 / max_fps.GetFloat() ) )
		return;

	WorldTime.TimeChanged( flCurTime );

	g_pCVar->RunFrame();

	g_pStudioMdlRenderer->RunFrame();

	GetSoundSystem()->RunFrame();

	RunFrame();
}
}
