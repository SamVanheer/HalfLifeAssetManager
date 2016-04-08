#include "CMainWindow.h"

#include <wx/display.h>
#include <wx/image.h>
#include <wx/cmdline.h>

#include <algorithm>
#include <vector>

#include "model/utility/OpenGL.h"
#include "ui/CwxOpenGL.h"

#include "CModelViewerApp.h"

wxIMPLEMENT_APP( CModelViewerApp );

CModelViewerApp* CModelViewerApp::m_pInstance = nullptr;

bool CModelViewerApp::OnInit()
{
	if( !wxApp::OnInit() )
		return false;

	m_pInstance = this;

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

	m_pPrimaryDisplay = new wxDisplay( 0 );

	wxArrayVideoModes modes = m_pPrimaryDisplay->GetModes();

	for( size_t uiIndex = 0; uiIndex < modes.Count(); ++uiIndex )
	{
		m_VideoModes.push_back( modes[ uiIndex ] );
	}

	m_VideoModes.erase( std::unique( m_VideoModes.begin(), m_VideoModes.end(), []( const wxVideoMode& lhs, const wxVideoMode& rhs )
	{
		return lhs.GetWidth() == rhs.GetWidth() && lhs.GetHeight() == lhs.GetHeight();
	}
	), m_VideoModes.end() );

	CMainWindow* pFrame = new CMainWindow();
	pFrame->Show( true );

	if( !m_szModel.IsEmpty() )
		pFrame->LoadModel( m_szModel );

	return true;
}

int CModelViewerApp::OnExit()
{
	wxOpenGL().Shutdown();

	delete m_pPrimaryDisplay;

	m_pInstance = nullptr;

	return wxApp::OnExit();
}

bool CModelViewerApp::OnCmdLineParsed( wxCmdLineParser& parser )
{
	//Last parameter is the model to load.
	if( parser.GetParamCount() > 0 )
		m_szModel = parser.GetParam( parser.GetParamCount() - 1 );

	return wxApp::OnCmdLineParsed( parser );
}

void CModelViewerApp::OnInitCmdLine( wxCmdLineParser& parser )
{
	wxApp::OnInitCmdLine( parser );

	//Note: this works by setting all available parameters in the order that they appear on the command line.
	//The model filename must be last for this to work with drag&drop.
	parser.AddParam( "Filename of the model to load on startup", wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL );
}