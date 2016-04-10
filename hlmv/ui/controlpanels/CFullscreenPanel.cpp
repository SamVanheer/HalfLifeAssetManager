#include <wx/gbsizer.h>

#include "ui/utility/wxUtil.h"

#include "CFullscreenPanel.h"

CFullscreenPanel::CFullscreenPanel( wxWindow* pParent, CHLMVSettings* const pSettings )
	: CBaseControlPanel( pParent, "Fullscreen", pSettings )
{
	wxWindow* const pElemParent = GetBox();

	wxStaticText* pResolution = new wxStaticText( pElemParent, wxID_ANY, "Resolution" );

	m_pResolution = new wxComboBox( pElemParent, wxID_ANY, "" );
	m_pResolution->SetEditable( false );

	//Try to get the display we're currently on.
	int iDisplay = wxDisplay::GetFromWindow( this );

	//Assume primary
	if( iDisplay == wxNOT_FOUND )
		iDisplay = 0;

	wxDisplay display( iDisplay );

	if( display.IsOk() )
	{
		std::vector<wxVideoMode> modes;

		wx::GetUniqueVideoModes( display, modes );

		for( const auto& mode : modes )
		{
			m_pResolution->Append( wxString::Format( "%d x %d", mode.GetWidth(), mode.GetHeight() ) );
		}
	}
	else
	{
		wxMessageBox( "Fullscreen support: Unable to get display for video modes query" );
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

//TODO: implement fullscreen mode