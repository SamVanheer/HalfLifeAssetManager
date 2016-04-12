#include <wx/gbsizer.h>

#include "hlmv/ui/CModelViewerApp.h"

#include "hlmv/ui/CFullscreenWindow.h"

#include "CFullscreenPanel.h"

wxBEGIN_EVENT_TABLE( CFullscreenPanel, CBaseControlPanel )
	EVT_BUTTON( wxID_FULLSCREEN_GO, CFullscreenPanel::GoFullscreen )
wxEND_EVENT_TABLE()

CFullscreenPanel::CFullscreenPanel( wxWindow* pParent, CHLMVSettings* const pSettings )
	: CBaseControlPanel( pParent, "Fullscreen", pSettings )
{
	wxWindow* const pElemParent = GetBox();

	m_pGoFullscreen = new wxButton( pElemParent, wxID_FULLSCREEN_GO, "Fullscreen!" );

	wxGridBagSizer* pSizer = new wxGridBagSizer( 5, 5 );

	pSizer->Add( m_pGoFullscreen, wxGBPosition( 0, 0 ), wxDefaultSpan, wxEXPAND );

	GetBoxSizer()->Add( pSizer );
}

CFullscreenPanel::~CFullscreenPanel()
{
}

void CFullscreenPanel::GoFullscreen( wxCommandEvent& event )
{
	if( wxGetApp().GetFullscreenWindow() )
	{
		wxMessageBox( "A fullscreen window is already open!" );
		return;
	}

	hlmv::CFullscreenWindow* pWindow = new hlmv::CFullscreenWindow( m_pSettings );
}