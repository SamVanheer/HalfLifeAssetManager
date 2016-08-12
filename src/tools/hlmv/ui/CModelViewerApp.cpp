#include "shared/Logging.h"

#include <wx/cmdline.h>

#include "game/entity/CEntityManager.h"
#include "game/entity/CBaseEntityList.h"

#include "CFullscreenWindow.h"
#include "CMainWindow.h"

#include "CModelViewerApp.h"

wxIMPLEMENT_APP( hlmv::CModelViewerApp );

namespace hlmv
{
namespace
{
//Use the default list class for now.
static CBaseEntityList g_EntityList;
}

bool CModelViewerApp::OnInit()
{
	SetAppDisplayName( HLMV_TITLE );
	SetLogFilename( HLMV_TITLE );

	return CBaseWXToolApp::OnInit();
}

void CModelViewerApp::OnInitCmdLine( wxCmdLineParser& parser )
{
	wxApp::OnInitCmdLine( parser );

	//Note: this works by setting all available parameters in the order that they appear on the command line.
	//The model filename must be last for this to work with drag&drop.
	parser.AddParam( "Filename of the model to load on startup", wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL );
}

bool CModelViewerApp::OnCmdLineParsed( wxCmdLineParser& parser )
{
	//Last parameter is the model to load.
	if( parser.GetParamCount() > 0 )
		m_szModel = parser.GetParam( parser.GetParamCount() - 1 );

	return wxApp::OnCmdLineParsed( parser );
}

void CModelViewerApp::SetMainWindow( CMainWindow* const pMainWindow )
{
	m_pMainWindow = pMainWindow;
}

void CModelViewerApp::SetFullscreenWindow( CFullscreenWindow* const pWindow )
{
	m_pFullscreenWindow = pWindow;
}

bool CModelViewerApp::PreRunApp()
{
	m_pState = new CHLMVState();
	m_pSettings = new CHLMVSettings( GetFileSystem() );

	SetToolIcon( wxICON( HLMV_ICON ) );

	SetEntityList( &g_EntityList );

	if( !EntityManager().OnMapBegin() )
	{
		FatalError( "Failed to start map\n" );
		return false;
	}

	//Must be called before we create the main window, since it accesses the window.
	UseMessagesWindow( true );

	if( !GetSettings()->Initialize( HLMV_SETTINGS_FILE ) )
	{
		return false;
	}

	m_pMainWindow = new hlmv::CMainWindow( this );

	m_pMainWindow->Show( true );

	if( !m_szModel.IsEmpty() )
		LoadModel( m_szModel );

	return true;
}

void CModelViewerApp::ShutdownApp()
{
	if( auto pSettings = GetSettings() )
	{
		pSettings->Shutdown( HLMV_SETTINGS_FILE );
	}

	if( m_pFullscreenWindow )
	{
		m_pFullscreenWindow = nullptr;
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

void CModelViewerApp::RunFrame()
{
	EntityManager().RunFrame();

	if( m_pFullscreenWindow )
		m_pFullscreenWindow->RunFrame();
	else if( m_pMainWindow )
		m_pMainWindow->RunFrame();
}

void CModelViewerApp::OnExit( const bool bMainWndClosed )
{
	if( bMainWndClosed )
		m_pMainWindow = nullptr;

	if( m_pFullscreenWindow )
	{
		m_pFullscreenWindow->Close( true );
		m_pFullscreenWindow = nullptr;
	}

	if( m_pMainWindow )
	{
		m_pMainWindow->Close( true );
		m_pMainWindow = nullptr;
	}
}

bool CModelViewerApp::LoadModel( const char* const pszFilename )
{
	return m_pMainWindow->LoadModel( pszFilename );
}

bool CModelViewerApp::PromptLoadModel()
{
	return m_pMainWindow->PromptLoadModel();
}

bool CModelViewerApp::SaveModel( const char* const pszFilename )
{
	return m_pMainWindow->SaveModel( pszFilename );
}

bool CModelViewerApp::PromptSaveModel()
{
	return m_pMainWindow->PromptSaveModel();
}

bool CModelViewerApp::LoadBackgroundTexture( const char* const pszFilename )
{
	return m_pMainWindow->LoadBackgroundTexture( pszFilename );
}

bool CModelViewerApp::PromptLoadBackgroundTexture()
{
	return m_pMainWindow->PromptLoadBackgroundTexture();
}

void CModelViewerApp::UnloadBackgroundTexture()
{
	m_pMainWindow->UnloadBackgroundTexture();
}

bool CModelViewerApp::LoadGroundTexture( const char* const pszFilename )
{
	return m_pMainWindow->LoadGroundTexture( pszFilename );
}

bool CModelViewerApp::PromptGroundTexture()
{
	return m_pMainWindow->PromptLoadGroundTexture();
}

void CModelViewerApp::UnloadGroundTexture()
{
	m_pMainWindow->UnloadGroundTexture();
}

void CModelViewerApp::SaveUVMap( const wxString& szFilename, const int iTexture )
{
	m_pMainWindow->SaveUVMap( szFilename, iTexture );
}
}
