#include "common/Logging.h"

#include "CMainWindow.h"
#include "CFullscreenWindow.h"

#include "CHLMV.h"

namespace hlmv
{
CHLMV::CHLMV()
	: CBaseTool( INIT_ALL, "Half-Life Model Viewer", wxICON( HLMV_ICON ), new CHLMVSettings() )
	, m_pState( new CHLMVState() )
{
	GetSettings()->SetSettingsListener( this );
}

CHLMV::~CHLMV()
{
	delete m_pState;
}

bool CHLMV::PostInitialize()
{
	//Must be called before we create the main window, since it accesses the window.
	UseMessagesWindow( true );

	if( !GetSettings()->Initialize( HLMV_SETTINGS_FILE ) )
	{
		return false;
	}

	m_pMainWindow = new hlmv::CMainWindow( this );

	m_pMainWindow->Show( true );

	StartTimer( GetSettings()->GetFPS() );

	return true;
}

void CHLMV::PreShutdown()
{
	GetSettings()->Shutdown( HLMV_SETTINGS_FILE );

	if( m_pFullscreenWindow )
	{
		m_pFullscreenWindow = nullptr;
	}

	if( m_pMainWindow )
	{
		m_pMainWindow = nullptr;
	}
}

void CHLMV::RunFrame()
{
	if( m_pFullscreenWindow )
		m_pFullscreenWindow->RunFrame();
	else if( m_pMainWindow )
		m_pMainWindow->RunFrame();
}

void CHLMV::OnExit( const bool bMainWndClosed )
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

void CHLMV::SetMainWindow( CMainWindow* const pMainWindow )
{
	m_pMainWindow = pMainWindow;
}

void CHLMV::SetFullscreenWindow( CFullscreenWindow* const pWindow )
{
	m_pFullscreenWindow = pWindow;
}

bool CHLMV::LoadModel( const char* const pszFilename )
{
	return m_pMainWindow->LoadModel( pszFilename );
}

bool CHLMV::PromptLoadModel()
{
	return m_pMainWindow->PromptLoadModel();
}

bool CHLMV::SaveModel( const char* const pszFilename )
{
	return m_pMainWindow->SaveModel( pszFilename );
}

bool CHLMV::PromptSaveModel()
{
	return m_pMainWindow->PromptSaveModel();
}

bool CHLMV::LoadBackgroundTexture( const char* const pszFilename )
{
	return m_pMainWindow->LoadBackgroundTexture( pszFilename );
}

bool CHLMV::PromptLoadBackgroundTexture()
{
	return m_pMainWindow->PromptLoadBackgroundTexture();
}

void CHLMV::UnloadBackgroundTexture()
{
	m_pMainWindow->UnloadBackgroundTexture();
}

bool CHLMV::LoadGroundTexture( const char* const pszFilename )
{
	return m_pMainWindow->LoadGroundTexture( pszFilename );
}

bool CHLMV::PromptGroundTexture()
{
	return m_pMainWindow->PromptLoadGroundTexture();
}

void CHLMV::UnloadGroundTexture()
{
	m_pMainWindow->UnloadGroundTexture();
}

void CHLMV::SaveUVMap( const wxString& szFilename, const int iTexture )
{
	m_pMainWindow->SaveUVMap( szFilename, iTexture );
}
}