#include <wx/gbsizer.h>
#include <wx/clrpicker.h>
#include <wx/statline.h>

#include "common/Platform.h"
#include "utility/Color.h"

#include "ui/utility/wxUtil.h"

#include "hlmv/settings/CHLMVSettings.h"

#include "CGeneralOptions.h"

namespace hlmv
{
wxBEGIN_EVENT_TABLE( CGeneralOptions, wxPanel )
	EVT_BUTTON( wxID_OPTIONS_GENERAL_DEFAULT_GROUND_COLOR, CGeneralOptions::SetDefaultColor )
	EVT_BUTTON( wxID_OPTIONS_GENERAL_DEFAULT_BACKGROUND_COLOR, CGeneralOptions::SetDefaultColor )
	EVT_BUTTON( wxID_OPTIONS_GENERAL_DEFAULT_CROSSHAIR_COLOR, CGeneralOptions::SetDefaultColor )
	EVT_BUTTON( wxID_OPTIONS_GENERAL_DEFAULT_LIGHT_COLOR, CGeneralOptions::SetDefaultColor )
	EVT_BUTTON( wxID_OPTIONS_GENERAL_DEFAULT_WIREFRAME_COLOR, CGeneralOptions::SetDefaultColor )
	EVT_BUTTON( wxID_OPTIONS_GENERAL_DEFAULT_FPS, CGeneralOptions::SetDefaultFPS )
wxEND_EVENT_TABLE()

CGeneralOptions::CGeneralOptions( wxWindow* pParent, CHLMVSettings* const pSettings )
	: wxPanel( pParent )
	, m_pSettings( pSettings )
{
	wxASSERT( pSettings );

	m_pGroundColor = new wxColourPickerCtrl( this, wxID_ANY, wx::ColorTowx( m_pSettings->GetGroundColor() ), wxDefaultPosition, wxDefaultSize );

	m_pBackgroundColor = new wxColourPickerCtrl( this, wxID_ANY, wx::ColorTowx( m_pSettings->GetBackgroundColor() ), wxDefaultPosition, wxDefaultSize );

	m_pCrosshairColor = new wxColourPickerCtrl( this, wxID_ANY, wx::ColorTowx( m_pSettings->GetCrosshairColor() ), wxDefaultPosition, wxDefaultSize );

	m_pLightColor = new wxColourPickerCtrl( this, wxID_ANY, wx::ColorTowx( m_pSettings->GetLightColor() ), wxDefaultPosition, wxDefaultSize );

	m_pWireframeColor = new wxColourPickerCtrl( this, wxID_ANY, wx::ColorTowx( m_pSettings->GetWireframeColor() ), wxDefaultPosition, wxDefaultSize );

	m_pFPS = new wxSlider( this, wxID_ANY, m_pSettings->GetFPS(), hlmv::CHLMVSettings::MIN_FPS, hlmv::CHLMVSettings::MAX_FPS, 
						   wxDefaultPosition, wxSize( 200, wxDefaultSize.GetHeight() ), wxSL_HORIZONTAL | wxSL_TOP | wxSL_LABELS );

	//Layout
	wxGridBagSizer* pSizer = new wxGridBagSizer( 5, 5 );

	int iRow = 0;

	const int iNumCols = 10;

	pSizer->Add( new wxStaticText( this, wxID_ANY, "Colors:" ), wxGBPosition( iRow++, 0 ), wxGBSpan( 1, iNumCols ) );

	pSizer->Add( new wxStaticLine( this ), wxGBPosition( iRow++, 0 ), wxGBSpan( 1, iNumCols ), wxEXPAND );

	pSizer->Add( new wxStaticText( this, wxID_ANY, "Ground Color:" ), wxGBPosition( iRow, 0 ), wxGBSpan( 1, 1 ) );
	pSizer->Add( m_pGroundColor, wxGBPosition( iRow, 1 ), wxGBSpan( 1, 1 ) );
	pSizer->Add( new wxButton( this, wxID_OPTIONS_GENERAL_DEFAULT_GROUND_COLOR, "Default" ), wxGBPosition( iRow++, 2 ), wxGBSpan( 1, 1 ) );

	pSizer->Add( new wxStaticText( this, wxID_ANY, "Background Color:" ), wxGBPosition( iRow, 0 ), wxGBSpan( 1, 1 ) );
	pSizer->Add( m_pBackgroundColor, wxGBPosition( iRow, 1 ), wxGBSpan( 1, 1 ) );
	pSizer->Add( new wxButton( this, wxID_OPTIONS_GENERAL_DEFAULT_BACKGROUND_COLOR, "Default" ), wxGBPosition( iRow++, 2 ), wxGBSpan( 1, 1 ) );

	pSizer->Add( new wxStaticText( this, wxID_ANY, "Crosshair Color:" ), wxGBPosition( iRow, 0 ), wxGBSpan( 1, 1 ) );
	pSizer->Add( m_pCrosshairColor, wxGBPosition( iRow, 1 ), wxGBSpan( 1, 1 ) );
	pSizer->Add( new wxButton( this, wxID_OPTIONS_GENERAL_DEFAULT_CROSSHAIR_COLOR, "Default" ), wxGBPosition( iRow++, 2 ), wxGBSpan( 1, 1 ) );

	pSizer->Add( new wxStaticText( this, wxID_ANY, "Light Color:" ), wxGBPosition( iRow, 0 ), wxGBSpan( 1, 1 ) );
	pSizer->Add( m_pLightColor, wxGBPosition( iRow, 1 ), wxGBSpan( 1, 1 ) );
	pSizer->Add( new wxButton( this, wxID_OPTIONS_GENERAL_DEFAULT_LIGHT_COLOR, "Default" ), wxGBPosition( iRow++, 2 ), wxGBSpan( 1, 1 ) );

	pSizer->Add( new wxStaticText( this, wxID_ANY, "Wireframe Color:" ), wxGBPosition( iRow, 0 ), wxGBSpan( 1, 1 ) );
	pSizer->Add( m_pWireframeColor, wxGBPosition( iRow, 1 ), wxGBSpan( 1, 1 ) );
	pSizer->Add( new wxButton( this, wxID_OPTIONS_GENERAL_DEFAULT_WIREFRAME_COLOR, "Default" ), wxGBPosition( iRow++, 2 ), wxGBSpan( 1, 1 ) );

	pSizer->Add( new wxStaticLine( this ), wxGBPosition( iRow++, 0 ), wxGBSpan( 1, iNumCols ), wxEXPAND );

	pSizer->Add( new wxStaticText( this, wxID_ANY, "FPS:" ), wxGBPosition( iRow, 0 ), wxGBSpan( 1, 1 ) );
	pSizer->Add( m_pFPS, wxGBPosition( iRow, 1 ), wxGBSpan( 1, 2 ) );
	pSizer->Add( new wxButton( this, wxID_OPTIONS_GENERAL_DEFAULT_FPS, "Default" ), wxGBPosition( iRow++, 3 ), wxGBSpan( 1, 1 ) );

	for( size_t uiIndex = 0; uiIndex < static_cast<size_t>( iNumCols ); ++uiIndex )
	{
		pSizer->AddGrowableCol( uiIndex );
	}

	this->SetSizer( pSizer );

	Initialize();
}

CGeneralOptions::~CGeneralOptions()
{
}

void CGeneralOptions::Save()
{
	m_pSettings->SetGroundColor( wx::wxToColor( m_pGroundColor->GetColour() ) );
	m_pSettings->SetBackgroundColor( wx::wxToColor( m_pBackgroundColor->GetColour() ) );
	m_pSettings->SetCrosshairColor( wx::wxToColor( m_pCrosshairColor->GetColour() ) );
	m_pSettings->SetLightColor( wx::wxToColor( m_pLightColor->GetColour() ) );
	m_pSettings->SetWireframeColor( wx::wxToColor( m_pWireframeColor->GetColour() ) );
	m_pSettings->SetFPS( m_pFPS->GetValue() );
}

void CGeneralOptions::Initialize()
{
	m_pGroundColor->SetColour( wx::ColorTowx( m_pSettings->GetGroundColor() ) );
	m_pBackgroundColor->SetColour( wx::ColorTowx( m_pSettings->GetBackgroundColor() ) );
	m_pCrosshairColor->SetColour( wx::ColorTowx( m_pSettings->GetCrosshairColor() ) );
	m_pLightColor->SetColour( wx::ColorTowx( m_pSettings->GetLightColor() ) );
	m_pWireframeColor->SetColour( wx::ColorTowx( m_pSettings->GetWireframeColor() ) );
	m_pFPS->SetValue( m_pSettings->GetFPS() );
}

void CGeneralOptions::SetDefaultColor( wxCommandEvent& event )
{
	Color color;
	wxColourPickerCtrl* pCtrl;

	switch( event.GetId() )
	{
	case wxID_OPTIONS_GENERAL_DEFAULT_GROUND_COLOR:
		{
			color = CHLMVSettings::DEFAULT_GROUND_COLOR;
			pCtrl = m_pGroundColor;
			break;
		}

	case wxID_OPTIONS_GENERAL_DEFAULT_BACKGROUND_COLOR:
		{
			color = CHLMVSettings::DEFAULT_BACKGROUND_COLOR;
			pCtrl = m_pBackgroundColor;
			break;
		}

	case wxID_OPTIONS_GENERAL_DEFAULT_CROSSHAIR_COLOR:
		{
			color = CHLMVSettings::DEFAULT_CROSSHAIR_COLOR;
			pCtrl = m_pCrosshairColor;
			break;
		}

	case wxID_OPTIONS_GENERAL_DEFAULT_LIGHT_COLOR:
		{
			color = CHLMVSettings::DEFAULT_LIGHT_COLOR;
			pCtrl = m_pLightColor;
			break;
		}

	case wxID_OPTIONS_GENERAL_DEFAULT_WIREFRAME_COLOR:
		{
			color = CHLMVSettings::DEFAULT_WIREFRAME_COLOR;
			pCtrl = m_pWireframeColor;
			break;
		}

	default:
		{
			wxMessageBox( wxString::Format( "CGeneralOptions::SetDefaultColor: Invalid id \"%d\"!\n", event.GetId() ) );
			return;
		}
	}

	pCtrl->SetColour( wx::ColorTowx( color ) );
}

void CGeneralOptions::SetDefaultFPS( wxCommandEvent& event )
{
	m_pFPS->SetValue( CHLMVSettings::DEFAULT_FPS );
}
}