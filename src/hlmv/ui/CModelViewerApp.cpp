#include "common/Logging.h"

#include "CMainWindow.h"
#include "CFullscreenWindow.h"

#include <wx/cmdline.h>

#include "soundsystem/CSoundSystem.h"

#include "CHLMV.h"

#include "CModelViewerApp.h"

wxIMPLEMENT_APP( CModelViewerApp );

bool CModelViewerApp::OnInit()
{
	if( !Initialize() )
	{
		Shutdown();
		return false;
	}

	return true;
}

int CModelViewerApp::OnExit()
{
	Shutdown();

	return wxApp::OnExit();
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

void CModelViewerApp::OnTimer( CTimer& timer )
{
	CwxBaseApp::OnTimer( timer );

	if( m_pListener )
		m_pListener->OnTimer( timer );
}

void CModelViewerApp::SetFullscreenWindow( hlmv::CFullscreenWindow* pWindow )
{
	m_pFullscreenWindow = pWindow;

	if( m_pFullscreenWindow )
	{
		m_pListener = m_pFullscreenWindow;
	}
	else
	{
		m_pListener = m_pMainWindow;
	}
}

void CModelViewerApp::ExitApp( const bool bMainWndClosed )
{
	CwxBaseApp::ExitApp( bMainWndClosed );

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

	//TODO: use log to file listener instead (prevents message boxes from popping up during shutdown)
	logging().SetLogListener( GetNullLogListener() );
}

bool CModelViewerApp::Initialize()
{
	if( !wxApp::OnInit() )
		return false;

	if( !InitApp( INIT_ALL, HLMV_TITLE ) )
		return false;

	if( !m_State.Initialize() )
	{
		wxMessageBox( "Failed to initialize settings", wxMessageBoxCaptionStr, wxOK | wxCENTRE | wxICON_ERROR );
		return false;
	}

	//Must be called before we create the main window, since it accesses the window.
	UseMessagesWindow( true );

	//TODO: add settings
	m_pHLMV = new hlmv::CHLMV( nullptr, &m_State );

	//TODO: use 2 step init to avoid a situation where the main window constructor tries to access the CHLMV instance.
	m_pMainWindow = new CMainWindow( m_pHLMV );

	m_pHLMV->SetMainWindow( m_pMainWindow );

	m_pMainWindow->Show( true );

	//TODO: tidy
	m_pListener = m_pMainWindow;

	StartTimer();

	if( !m_szModel.IsEmpty() )
		m_pMainWindow->LoadModel( m_szModel );

	return true;
}

void CModelViewerApp::Shutdown()
{
	if( m_pFullscreenWindow )
	{
		m_pFullscreenWindow = nullptr;
	}

	if( m_pMainWindow )
	{
		m_pMainWindow = nullptr;
	}

	if( m_pHLMV )
	{
		delete m_pHLMV;
		m_pHLMV = nullptr;
	}

	m_State.Shutdown();

	ShutdownApp();
}