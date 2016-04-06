#include <wx/gbsizer.h>

#include "hlmv/ui/CModelViewerApp.h"

#include "CFullscreenPanel.h"

CFullscreenPanel::CFullscreenPanel( wxWindow* pParent )
	: CBaseControlPanel( pParent, "Fullscreen" )
{
	wxWindow* const pElemParent = GetBox();

	wxStaticText* pResolution = new wxStaticText( pElemParent, wxID_ANY, "Resolution" );

	m_pResolution = new wxComboBox( pElemParent, wxID_ANY, "" );
	m_pResolution->SetEditable( false );

	const auto& modes = CModelViewerApp::GetApp().GetVideoModes();

	for( const auto& mode : modes )
	{
		m_pResolution->Append( wxString::Format( "%d x %d", mode.GetWidth(), mode.GetHeight() ) );
	}

	m_pResolution->Select( 0 );

	m_pGoFullscreen = new wxButton( pElemParent, wxID_ANY, "Fullscreen!" );

	wxGridBagSizer* pSizer = new wxGridBagSizer( 5, 5 );

	pSizer->Add( pResolution, wxGBPosition( 0, 0 ), wxDefaultSpan, wxEXPAND );
	pSizer->Add( m_pResolution, wxGBPosition( 1, 0 ), wxDefaultSpan, wxEXPAND );

	pSizer->Add( m_pGoFullscreen, wxGBPosition( 1, 1 ), wxDefaultSpan, wxEXPAND );

	GetBoxSizer()->Add( pSizer );
}

CFullscreenPanel::~CFullscreenPanel()
{
}

void CFullscreenPanel::ModelChanged( const StudioModel& model )
{
	//TODO: fill info
}