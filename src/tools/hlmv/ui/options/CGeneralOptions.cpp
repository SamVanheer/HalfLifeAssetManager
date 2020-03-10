#include <wx/gbsizer.h>
#include <wx/clrpicker.h>
#include <wx/statline.h>

#include "shared/Platform.h"
#include "utility/Color.h"

#include "wx/utility/wxUtil.h"

#include "../../settings/CHLMVSettings.h"

#include "cvar/CVar.h"
#include "cvar/CVarUtils.h"

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
	EVT_BUTTON( wxID_OPTIONS_GENERAL_DEFAULT_FLOOR_LENGTH, CGeneralOptions::SetDefaultFloorLength )
wxEND_EVENT_TABLE()

CGeneralOptions::CGeneralOptions( wxWindow* pParent, CHLMVSettings* const pSettings )
	: wxPanel( pParent )
	, m_pSettings( pSettings )
{
	wxASSERT( pSettings );

	m_pPowerOf2Textures = new wxCheckBox(this, wxID_ANY, "Rescale textures to power of 2" );
	m_pInvertHorizontalDragging = new wxCheckBox(this, wxID_ANY, "Invert horizontal dragging direction");
	m_pInvertVerticalDragging = new wxCheckBox(this, wxID_ANY, "Invert vertical dragging direction");

	m_pGroundColor = new wxColourPickerCtrl( this, wxID_ANY, wx::ColorTowx( m_pSettings->GetGroundColor() ), wxDefaultPosition, wxDefaultSize );

	m_pBackgroundColor = new wxColourPickerCtrl( this, wxID_ANY, wx::ColorTowx( m_pSettings->GetBackgroundColor() ), wxDefaultPosition, wxDefaultSize );

	m_pCrosshairColor = new wxColourPickerCtrl( this, wxID_ANY, wx::ColorTowx( m_pSettings->GetCrosshairColor() ), wxDefaultPosition, wxDefaultSize );

	m_pLightColor = new wxColourPickerCtrl( this, wxID_ANY, wx::ColorTowx( CHLMVSettings::DEFAULT_LIGHT_COLOR ), wxDefaultPosition, wxDefaultSize );

	m_pWireframeColor = new wxColourPickerCtrl( this, wxID_ANY, wx::ColorTowx( CHLMVSettings::DEFAULT_WIREFRAME_COLOR ), wxDefaultPosition, wxDefaultSize );

	m_pFPS = new wxSlider( this, wxID_ANY, 60, hlmv::CHLMVSettings::MIN_FPS, hlmv::CHLMVSettings::MAX_FPS, 
						   wxDefaultPosition, wxSize( 200, wxDefaultSize.GetHeight() ), wxSL_HORIZONTAL | wxSL_TOP | wxSL_LABELS );

	m_pFloorLength = new wxSlider( this, wxID_ANY, m_pSettings->GetFloorLength(), CHLMVSettings::MIN_FLOOR_LENGTH, CHLMVSettings::MAX_FLOOR_LENGTH,
								   wxDefaultPosition, wxSize( 200, wxDefaultSize.GetHeight() ), wxSL_HORIZONTAL | wxSL_TOP | wxSL_LABELS );

	//Layout
	wxGridBagSizer* pSizer = new wxGridBagSizer( 5, 5 );

	int iRow = 0;

	const int iNumCols = 10;

	pSizer->Add( new wxStaticText( this, wxID_ANY, "General:" ), wxGBPosition( iRow++, 0 ), wxGBSpan( 1, iNumCols ) );

	pSizer->Add( new wxStaticLine( this ), wxGBPosition( iRow++, 0 ), wxGBSpan( 1, iNumCols ), wxEXPAND );

	pSizer->Add( m_pPowerOf2Textures, wxGBPosition( iRow++, 0), wxGBSpan( 1, iNumCols ), wxEXPAND );
	pSizer->Add(m_pInvertHorizontalDragging, wxGBPosition(iRow++, 0), wxGBSpan(1, iNumCols), wxEXPAND);
	pSizer->Add(m_pInvertVerticalDragging, wxGBPosition(iRow++, 0), wxGBSpan(1, iNumCols), wxEXPAND);

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

	const int iDefaultCol = iNumCols - 5;

	pSizer->Add( new wxStaticText( this, wxID_ANY, "FPS:" ), wxGBPosition( iRow, 0 ), wxGBSpan( 1, 1 ) );
	pSizer->Add( m_pFPS, wxGBPosition( iRow, 1 ), wxGBSpan( 1, iDefaultCol - 1 ), wxEXPAND );
	pSizer->Add( new wxButton( this, wxID_OPTIONS_GENERAL_DEFAULT_FPS, "Default" ), wxGBPosition( iRow++, iDefaultCol ), wxGBSpan( 1, 1 ) );

	pSizer->Add( new wxStaticText( this, wxID_ANY, "Floor Length:" ), wxGBPosition( iRow, 0 ), wxGBSpan( 1, 1 ) );
	pSizer->Add( m_pFloorLength, wxGBPosition( iRow, 1 ), wxGBSpan( 1, iDefaultCol - 1 ), wxEXPAND );
	pSizer->Add( new wxButton( this, wxID_OPTIONS_GENERAL_DEFAULT_FLOOR_LENGTH, "Default" ), wxGBPosition( iRow++, iDefaultCol ), wxGBSpan( 1, 1 ) );

	for( size_t uiIndex = 0; uiIndex < static_cast<size_t>( iNumCols ); ++uiIndex )
	{
		pSizer->AddGrowableCol( uiIndex );
	}

	this->SetSizer( pSizer );

	Initialize();

	g_pCVar->InstallGlobalCVarHandler( this );
}

CGeneralOptions::~CGeneralOptions()
{
	g_pCVar->RemoveGlobalCVarHandler( this );
}

void CGeneralOptions::Save()
{
	auto pPowerOf2 = static_cast<cvar::CCVar*>( g_pCVar->FindCommand( "r_powerof2textures") );

	pPowerOf2->SetBool( m_pPowerOf2Textures ->GetValue() );

	m_pSettings->SetInvertHoritonzalDraggingDirection(m_pInvertHorizontalDragging->GetValue());
	m_pSettings->SetInvertVerticalDraggingDirection(m_pInvertVerticalDragging->GetValue());

	m_pSettings->SetGroundColor( wx::wxToColor( m_pGroundColor->GetColour() ) );
	m_pSettings->SetBackgroundColor( wx::wxToColor( m_pBackgroundColor->GetColour() ) );
	m_pSettings->SetCrosshairColor( wx::wxToColor( m_pCrosshairColor->GetColour() ) );

	Color color = wx::wxToColor( m_pLightColor->GetColour() );

	cvar::CCVar* pLightingR;
	cvar::CCVar* pLightingG;
	cvar::CCVar* pLightingB;

	if( cvar::GetColorCVars( "r_lighting", &pLightingR, &pLightingG, &pLightingB ) )
	{
		pLightingR->SetInt( color.GetRed() );
		pLightingG->SetInt( color.GetGreen() );
		pLightingB->SetInt( color.GetBlue() );
	}

	color = wx::wxToColor( m_pWireframeColor->GetColour() );

	if( cvar::GetColorCVars( "r_wireframecolor", &pLightingR, &pLightingG, &pLightingB ) )
	{
		pLightingR->SetInt( color.GetRed() );
		pLightingG->SetInt( color.GetGreen() );
		pLightingB->SetInt( color.GetBlue() );
	}

	g_pCVar->SetCVarFloat( "max_fps", m_pFPS->GetValue() );
	m_pSettings->SetFloorLength( m_pFloorLength->GetValue() );
}

void CGeneralOptions::Initialize()
{
	auto pPowerOf2 = static_cast< cvar::CCVar* >( g_pCVar->FindCommand( "r_powerof2textures" ) );

	m_pPowerOf2Textures->SetValue( pPowerOf2->GetBool() );
	m_pInvertHorizontalDragging->SetValue(m_pSettings->InvertHorizontalDraggingDirection());
	m_pInvertVerticalDragging->SetValue(m_pSettings->InvertVerticalDraggingDirection());

	m_pGroundColor->SetColour( wx::ColorTowx( m_pSettings->GetGroundColor() ) );
	m_pBackgroundColor->SetColour( wx::ColorTowx( m_pSettings->GetBackgroundColor() ) );
	m_pCrosshairColor->SetColour( wx::ColorTowx( m_pSettings->GetCrosshairColor() ) );

	cvar::CCVar* pLightingR;
	cvar::CCVar* pLightingG;
	cvar::CCVar* pLightingB;

	if( cvar::GetColorCVars( "r_lighting", &pLightingR, &pLightingG, &pLightingB ) )
	{
		m_pLightColor->SetColour( wx::ColorTowx( Color( pLightingR->GetInt(), pLightingG->GetInt(), pLightingB->GetInt() ) ) );
	}

	if( cvar::GetColorCVars( "r_wireframecolor", &pLightingR, &pLightingG, &pLightingB ) )
	{
		m_pWireframeColor->SetColour( wx::ColorTowx( Color( pLightingR->GetInt(), pLightingG->GetInt(), pLightingB->GetInt() ) ) );
	}

	m_pFPS->SetValue( g_pCVar->GetCVarFloat( "max_fps" ) );
	m_pFloorLength->SetValue( m_pSettings->GetFloorLength() );
}

void CGeneralOptions::HandleCVar( cvar::CCVar& cvar, const char* pszOldValue, float flOldValue )
{
	//Update the value in real-time.
	if( strncmp( "r_lighting_", cvar.GetName(), 11 ) == 0 )
	{
		cvar::CCVar* pLightingR;
		cvar::CCVar* pLightingG;
		cvar::CCVar* pLightingB;

		if( cvar::GetColorCVars( "r_lighting", &pLightingR, &pLightingG, &pLightingB ) )
		{
			m_pLightColor->SetColour( wx::ColorTowx( Color( pLightingR->GetInt(), pLightingG->GetInt(), pLightingB->GetInt() ) ) );
		}
	}
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

void CGeneralOptions::SetDefaultFloorLength( wxCommandEvent& event )
{
	m_pFloorLength->SetValue( CHLMVSettings::DEFAULT_FLOOR_LENGTH );
}
}