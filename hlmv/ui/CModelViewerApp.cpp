#include "CMainWindow.h"
#include "CFullscreenWindow.h"

#include <wx/image.h>
#include <wx/cmdline.h>

#include <algorithm>
#include <vector>

#include "model/utility/OpenGL.h"
#include "ui/CwxOpenGL.h"

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

void CModelViewerApp::SetFullscreenWindow( hlmv::CFullscreenWindow* pWindow )
{
	m_pFullscreenWindow = pWindow;

	if( m_pFullscreenWindow )
	{
		m_pTimer->SetListener( m_pFullscreenWindow );
	}
	else
	{
		m_pTimer->SetListener( m_pMainWindow );
	}
}

void CModelViewerApp::ExitApp( const bool bMainWndClosed )
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

	m_pTimer->SetListener( nullptr );
}

bool CModelViewerApp::Initialize()
{
	if( !wxApp::OnInit() )
		return false;

	SetAppDisplayName( HLMV_TITLE );

	//Set up OpenGL parameters.
	//TODO: move to common base class
	{
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

	wxInitAllImageHandlers();

	m_pMainWindow = new CMainWindow();

	m_pMainWindow->Show( true );

	m_pTimer = new CTimer( m_pMainWindow );

	//60 FPS
	m_pTimer->Start( ( 1 / 60.0 ) * 1000 );

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

	if( m_pTimer )
	{
		delete m_pTimer;
		m_pTimer = nullptr;
	}

	wxOpenGL().Shutdown();

	CwxOpenGL::DestroyInstance();
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