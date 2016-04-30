#include "shared/Logging.h"

#include <wx/cmdline.h>

#include "CSpriteViewer.h"

#include "CSpriteViewerApp.h"

wxIMPLEMENT_APP( CSpriteViewerApp );

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

bool CSpriteViewerApp::PostInitialize()
{
	if( !m_szSprite.IsEmpty() )
		GetTool()->LoadSprite( m_szSprite );

	return true;
}