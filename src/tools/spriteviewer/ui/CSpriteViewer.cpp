#include "shared/Logging.h"

#include "wxSpriteViewer.h"

#include "CMainWindow.h"

#include "CSpriteViewer.h"

namespace sprview
{
CSpriteViewer::CSpriteViewer()
	: CBaseTool( "Sprite Viewer", wxICON( HLMV_ICON ) )
	, m_pState( new CSpriteViewerState() )
{
}

CSpriteViewer::~CSpriteViewer()
{
	delete m_pState;
}

settings::CBaseSettings* CSpriteViewer::CreateSettings()
{
	return new CSpriteViewerSettings( GetFileSystem() );
}

bool CSpriteViewer::PostInitialize()
{
	GetSettings()->SetSettingsListener( this );

	//Must be called before we create the main window, since it accesses the window.
	UseMessagesWindow( true );

	if( !GetSettings()->Initialize( SPRITEVIEWER_SETTINGS_FILE ) )
	{
		return false;
	}

	m_pMainWindow = new sprview::CMainWindow( this );

	m_pMainWindow->Show( true );

	StartTimer( GetSettings()->GetFPS() );

	return true;
}

void CSpriteViewer::PreShutdown()
{
	if( auto pSettings = GetSettings() )
	{
		pSettings->Shutdown( SPRITEVIEWER_SETTINGS_FILE );
	}

	if( m_pMainWindow )
	{
		m_pMainWindow = nullptr;
	}
}

void CSpriteViewer::RunFrame()
{
	if( m_pMainWindow )
		m_pMainWindow->RunFrame();
}

void CSpriteViewer::OnExit( const bool bMainWndClosed )
{
	if( bMainWndClosed )
		m_pMainWindow = nullptr;

	if( m_pMainWindow )
	{
		m_pMainWindow->Close( true );
		m_pMainWindow = nullptr;
	}
}

void CSpriteViewer::SetMainWindow( CMainWindow* const pMainWindow )
{
	m_pMainWindow = pMainWindow;
}

bool CSpriteViewer::LoadSprite( const wxString& szFilename )
{
	return false;
}
}