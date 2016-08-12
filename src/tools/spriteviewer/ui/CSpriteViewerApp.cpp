#include "shared/Logging.h"

#include <wx/cmdline.h>

#include "engine/shared/renderer/sprite/ISpriteRenderer.h"

#include "game/entity/CEntityManager.h"
#include "game/entity/CBaseEntityList.h"

#include "CMainWindow.h"

#include "wxSpriteViewer.h"

#include "CSpriteViewerApp.h"

sprite::ISpriteRenderer* g_pSpriteRenderer = nullptr;

wxIMPLEMENT_APP( sprview::CSpriteViewerApp );

namespace sprview
{
namespace
{
//Use the default list class for now.
static CBaseEntityList g_EntityList;
}

bool CSpriteViewerApp::OnInit()
{
	SetAppDisplayName( SPRITEVIEWER_TITLE );
	SetLogFilename( SPRITEVIEWER_TITLE );

	return CBaseWXToolApp::OnInit();
}

void CSpriteViewerApp::OnInitCmdLine( wxCmdLineParser& parser )
{
	wxApp::OnInitCmdLine( parser );

	//Note: this works by setting all available parameters in the order that they appear on the command line.
	//The model filename must be last for this to work with drag&drop.
	parser.AddParam( "Filename of the sprite to load on startup", wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL );
}

bool CSpriteViewerApp::OnCmdLineParsed( wxCmdLineParser& parser )
{
	//Last parameter is the model to load.
	if( parser.GetParamCount() > 0 )
		m_szSprite = parser.GetParam( parser.GetParamCount() - 1 );

	return wxApp::OnCmdLineParsed( parser );
}

void CSpriteViewerApp::SetMainWindow( CMainWindow* const pMainWindow )
{
	m_pMainWindow = pMainWindow;
}

bool CSpriteViewerApp::LoadSprite( const wxString& szFilename )
{
	return false;
}

bool CSpriteViewerApp::Connect( const CreateInterfaceFn* pFactories, const size_t uiNumFactories )
{
	if( !CBaseWXToolApp::Connect( pFactories, uiNumFactories ) )
		return false;

	if( !LoadAndCheckInterfaces( pFactories, uiNumFactories,
								 IFace( ISPRITERENDERER_NAME, g_pSpriteRenderer, "Sprite Renderer" ) ) )
	{
		return false;
	}

	return true;
}

bool CSpriteViewerApp::PreRunApp()
{
	m_pState = new sprview::CSpriteViewerState();
	m_pSettings = new sprview::CSpriteViewerSettings( GetFileSystem() );

	SetToolIcon( wxICON( HLMV_ICON ) );

	SetEntityList( &g_EntityList );

	if( !EntityManager().OnMapBegin() )
	{
		FatalError( "Failed to initialize start map", wxMessageBoxCaptionStr, wxOK | wxCENTRE | wxICON_ERROR );
		return false;
	}

	//Must be called before we create the main window, since it accesses the window.
	UseMessagesWindow( true );

	if( !GetSettings()->Initialize( SPRITEVIEWER_SETTINGS_FILE ) )
	{
		return false;
	}

	m_pMainWindow = new sprview::CMainWindow( this );
	
	m_pMainWindow->Show( true );

	if( !m_szSprite.IsEmpty() )
		LoadSprite( m_szSprite );

	return true;
}

void CSpriteViewerApp::ShutdownApp()
{
	if( auto pSettings = GetSettings() )
	{
		pSettings->Shutdown( SPRITEVIEWER_SETTINGS_FILE );
	}

	if( m_pMainWindow )
	{
		m_pMainWindow = nullptr;
	}

	if( EntityManager().IsMapRunning() )
	{
		EntityManager().OnMapEnd();
	}

	EntityManager().Shutdown();

	if( m_pSettings )
	{
		delete m_pSettings;
		m_pSettings = nullptr;
	}

	if( m_pState )
	{
		delete m_pState;
		m_pState = nullptr;
	}

	CBaseWXToolApp::ShutdownApp();
}

void CSpriteViewerApp::RunFrame()
{
	EntityManager().RunFrame();

	if( m_pMainWindow )
		m_pMainWindow->RunFrame();
}

void CSpriteViewerApp::OnExit( const bool bMainWndClosed )
{
	if( bMainWndClosed )
		m_pMainWindow = nullptr;

	if( m_pMainWindow )
	{
		m_pMainWindow->Close( true );
		m_pMainWindow = nullptr;
	}
}
}
