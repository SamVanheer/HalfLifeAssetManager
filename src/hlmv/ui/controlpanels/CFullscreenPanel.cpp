#include <wx/gbsizer.h>

#include "hlmv/ui/CFullscreenWindow.h"

#include "hlmv/ui/CHLMV.h"

#include "CFullscreenPanel.h"

namespace hlmv
{
wxBEGIN_EVENT_TABLE( CFullscreenPanel, CBaseControlPanel )
	EVT_BUTTON( wxID_FULLSCREEN_GO, CFullscreenPanel::GoFullscreen )
wxEND_EVENT_TABLE()

CFullscreenPanel::CFullscreenPanel( wxWindow* pParent, CHLMV* const pHLMV )
	: CBaseControlPanel( pParent, "Fullscreen", pHLMV )
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
	if( m_pHLMV->GetFullscreenWindow() )
	{
		wxMessageBox( "A fullscreen window is already open!" );
		return;
	}

	CFullscreenWindow* pWindow = new CFullscreenWindow( m_pHLMV );
}
}