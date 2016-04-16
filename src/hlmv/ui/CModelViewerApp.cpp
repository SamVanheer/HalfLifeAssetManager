#include "common/Logging.h"

#include <wx/cmdline.h>

#include "CHLMV.h"

#include "CModelViewerApp.h"

wxIMPLEMENT_APP( CModelViewerApp );

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

bool CModelViewerApp::PostInitialize()
{
	if( !m_szModel.IsEmpty() )
		GetTool()->LoadModel( m_szModel );

	return true;
}