#include <wx/gbsizer.h>

#include "model/options/COptions.h"

#include "CTexturesPanel.h"

CTexturesPanel::CTexturesPanel( wxWindow* pParent )
	: CBaseControlPanel( pParent, "Textures" )
{
	//Helps catch errors if we miss one.
	memset( m_pCheckBoxes, 0, sizeof( m_pCheckBoxes ) );

	wxWindow* const pElemParent = GetBox();

	m_pTextureSize = new wxStaticText( pElemParent, wxID_ANY, "Texture (size: Undefined x Undefined)" );

	m_pTexture = new wxComboBox( pElemParent, wxID_ANY, "" );

	wxStaticText* pScaleTextureView = new wxStaticText( pElemParent, wxID_ANY, "Scale Texture View (Undefinedx)" );

	m_pScaleTextureView = new wxSlider( pElemParent, wxID_ANY, TEXTUREVIEW_SLIDER_DEFAULT, TEXTUREVIEW_SLIDER_MIN, TEXTUREVIEW_SLIDER_MAX );

	m_pCheckBoxes[ CheckBox::CHROME ]				= new wxCheckBox( pElemParent, wxID_ANY, "Chrome" );
	m_pCheckBoxes[ CheckBox::SHOW_UV_MAP ]			= new wxCheckBox( pElemParent, wxID_ANY, "Show UV Map" );
	m_pCheckBoxes[ CheckBox::ADDITIVE ]				= new wxCheckBox( pElemParent, wxID_ANY, "Additive" );
	m_pCheckBoxes[ CheckBox::OVERLAY_UV_MAP ]		= new wxCheckBox( pElemParent, wxID_ANY, "Overlay UV Map" );
	m_pCheckBoxes[ CheckBox::TRANSPARENT ]			= new wxCheckBox( pElemParent, wxID_ANY, "Transparent" );
	m_pCheckBoxes[ CheckBox::ANTI_ALIAS_LINES ]		= new wxCheckBox( pElemParent, wxID_ANY, "Anti-alias Lines" );

	m_pMesh = new wxComboBox( pElemParent, wxID_ANY, "" );

	m_pImportTexButton = new wxButton( pElemParent, wxID_ANY, "Import Texture" );
	m_pExportTexButton = new wxButton( pElemParent, wxID_ANY, "Export Texture" );
	m_pExportUVButton = new wxButton( pElemParent, wxID_ANY, "Export UV Map" );

	//Layout
	wxGridBagSizer* pSizer = new wxGridBagSizer( 5, 5 );

	pSizer->Add( m_pTextureSize, wxGBPosition( 0, 0 ), wxDefaultSpan, wxEXPAND | wxALIGN_CENTER_VERTICAL );
	pSizer->Add( m_pTexture, wxGBPosition( 1, 0 ), wxDefaultSpan, wxEXPAND );
	pSizer->Add( pScaleTextureView, wxGBPosition( 3, 0 ), wxDefaultSpan, wxEXPAND );
	pSizer->Add( m_pScaleTextureView, wxGBPosition( 4, 0 ), wxDefaultSpan, wxEXPAND );

	for( size_t uiIndex = CheckBox::FIRST; uiIndex < CheckBox::COUNT; ++uiIndex )
	{
		if( !m_pCheckBoxes[ uiIndex ] )
			wxLogError( "CTexturesPanel::CTexturesPanel: Null checkbox %u!\n", uiIndex );

		pSizer->Add( m_pCheckBoxes[ uiIndex ], wxGBPosition( uiIndex / NUM_CHECKBOXES_PER_ROW, 1 + ( uiIndex % NUM_CHECKBOXES_PER_ROW ) ), wxDefaultSpan, wxEXPAND );
	}

	pSizer->Add( m_pMesh, wxGBPosition( 4, 1 ), wxGBSpan( 1, 2 ), wxEXPAND );

	pSizer->Add( m_pImportTexButton, wxGBPosition( 0, 3 ), wxDefaultSpan, wxEXPAND );
	pSizer->Add( m_pExportTexButton, wxGBPosition( 1, 3 ), wxDefaultSpan, wxEXPAND );
	pSizer->Add( m_pExportUVButton, wxGBPosition( 2, 3 ), wxDefaultSpan, wxEXPAND );

	GetBoxSizer()->Add( pSizer );
}

CTexturesPanel::~CTexturesPanel()
{
}

void CTexturesPanel::PanelActivated()
{
	Options.showTexture = true;
}

void CTexturesPanel::PanelDeactivated()
{
	Options.showTexture = false;
}

void CTexturesPanel::ModelChanged( const StudioModel& model )
{
	//TODO: fill info
}