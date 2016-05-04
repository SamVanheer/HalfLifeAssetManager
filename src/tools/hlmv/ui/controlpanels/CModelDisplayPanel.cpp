#include <wx/gbsizer.h>

#include "../CHLMV.h"

#include "cvar/CCVarSystem.h"

#include "CModelDisplayPanel.h"

namespace hlmv
{
wxBEGIN_EVENT_TABLE( CModelDisplayPanel, CBaseControlPanel )
	EVT_COMBOBOX( wxID_MDLDISP_RENDERMODE, CModelDisplayPanel::RenderModeChanged )
	EVT_SLIDER( wxID_MDLDISP_OPACITY, CModelDisplayPanel::OpacityChanged )
	EVT_CHECKBOX( wxID_MDLDISP_CHECKBOX, CModelDisplayPanel::CheckBoxChanged )
	EVT_BUTTON( wxID_MDLDISP_SCALEMESH, CModelDisplayPanel::ScaleMesh )
	EVT_BUTTON( wxID_MDLDISP_SCALEBONES, CModelDisplayPanel::ScaleBones )
	EVT_CHECKBOX( wxID_MDLDISP_MIRROR, CModelDisplayPanel::OnMirrorAxis )
wxEND_EVENT_TABLE()

//Client data for the mirror checkboxes
static const size_t MIRROR[ 3 ] = { 0, 1, 2 };

CModelDisplayPanel::CModelDisplayPanel( wxWindow* pParent, CHLMV* const pHLMV )
	: CBaseControlPanel( pParent, "Model Display", pHLMV )
{
	//Helps catch errors if we miss one.
	memset( m_pCheckBoxes, 0, sizeof( m_pCheckBoxes ) );

	wxWindow* const pElemParent = GetElementParent();

	wxStaticText* pRenderMode = new wxStaticText( pElemParent, wxID_ANY, "Render mode:" );

	m_pOpacity = new wxStaticText( pElemParent, wxID_ANY, "Opacity: Undefined%" );

	m_pOpacitySlider = new wxSlider( pElemParent, wxID_MDLDISP_OPACITY, OPACITY_DEFAULT, OPACITY_MIN, OPACITY_MAX );

	m_pRenderMode = new wxComboBox( pElemParent, wxID_MDLDISP_RENDERMODE );

	for( int iRenderMode = static_cast<int>( RenderMode::FIRST ); iRenderMode < static_cast<int>( RenderMode::COUNT ); ++iRenderMode )
	{
		m_pRenderMode->Append( RenderModeToString( static_cast<RenderMode>( iRenderMode ) ) );
	}

	m_pRenderMode->SetSelection( static_cast<int>( RenderMode::TEXTURE_SHADED ) );

	m_pRenderMode->SetEditable( false );

	wxPanel* pCheckBoxPanel = new wxPanel( pElemParent );

	m_pCheckBoxes[ CheckBox::SHOW_HITBOXES ]		= new wxCheckBox( pCheckBoxPanel, wxID_MDLDISP_CHECKBOX, "Show Hit Boxes" );
	m_pCheckBoxes[ CheckBox::SHOW_GROUND ]			= new wxCheckBox( pCheckBoxPanel, wxID_MDLDISP_CHECKBOX, "Show Ground" );
	m_pCheckBoxes[ CheckBox::SHOW_BONES ]			= new wxCheckBox( pCheckBoxPanel, wxID_MDLDISP_CHECKBOX, "Show Bones" );
	m_pCheckBoxes[ CheckBox::MIRROR_ON_GROUND ]		= new wxCheckBox( pCheckBoxPanel, wxID_MDLDISP_CHECKBOX, "Mirror Model On Ground" );
	m_pCheckBoxes[ CheckBox::SHOW_ATTACHMENTS ]		= new wxCheckBox( pCheckBoxPanel, wxID_MDLDISP_CHECKBOX, "Show Attachments" );
	m_pCheckBoxes[ CheckBox::SHOW_BACKGROUND ]		= new wxCheckBox( pCheckBoxPanel, wxID_MDLDISP_CHECKBOX, "Show Background" );
	m_pCheckBoxes[ CheckBox::SHOW_EYE_POSITION ]	= new wxCheckBox( pCheckBoxPanel, wxID_MDLDISP_CHECKBOX, "Show Eye Position" );
	m_pCheckBoxes[ CheckBox::WIREFRAME_OVERLAY ]	= new wxCheckBox( pCheckBoxPanel, wxID_MDLDISP_CHECKBOX, "Wireframe Overlay" );
	m_pCheckBoxes[ CheckBox::BACKFACE_CULLING ]		= new wxCheckBox( pCheckBoxPanel, wxID_MDLDISP_CHECKBOX, "Backface Culling" );

	m_pCheckBoxes[ CheckBox::BACKFACE_CULLING ]->SetValue( true );

	m_pMeshScale = new wxTextCtrl( pElemParent, wxID_ANY, "1.0" );
	m_pMeshScaleButton = new wxButton( pElemParent, wxID_MDLDISP_SCALEMESH, "Scale Mesh" );

	m_pBonesScale = new wxTextCtrl( pElemParent, wxID_ANY, "1.0" );
	m_pBonesScaleButton = new wxButton( pElemParent, wxID_MDLDISP_SCALEBONES, "Scale Bones" );

	m_pMirror[ 0 ] = new wxCheckBox( pElemParent, wxID_MDLDISP_MIRROR, "Mirror on X axis" );
	m_pMirror[ 1 ] = new wxCheckBox( pElemParent, wxID_MDLDISP_MIRROR, "Mirror on Y axis" );
	m_pMirror[ 2 ] = new wxCheckBox( pElemParent, wxID_MDLDISP_MIRROR, "Mirror on Z axis" );

	for( size_t uiIndex = 0; uiIndex < 3; ++uiIndex )
	{
		m_pMirror[ uiIndex ]->SetClientData( const_cast<size_t*>( &MIRROR[ uiIndex ] ) );
	}

	//Layout
	wxGridBagSizer* pControlsSizer = new wxGridBagSizer( CONTROLS_ROW_GAP, CONTROLS_COL_GAP );

	pControlsSizer->Add( pRenderMode, wxGBPosition( 0, 0 ), wxDefaultSpan, wxEXPAND );
	pControlsSizer->Add( m_pRenderMode, wxGBPosition( 1, 0 ), wxDefaultSpan, wxEXPAND );

	pControlsSizer->Add( m_pOpacity, wxGBPosition( 2, 0 ), wxDefaultSpan, wxEXPAND );
	pControlsSizer->Add( m_pOpacitySlider, wxGBPosition( 3, 0 ), wxDefaultSpan, wxEXPAND );

	wxGridBagSizer* pCheckBoxSizer = new wxGridBagSizer( 5, 5 );

	for( size_t uiIndex = CheckBox::FIRST; uiIndex < CheckBox::COUNT; ++uiIndex )
	{
		if( !m_pCheckBoxes[ uiIndex ] )
			wxLogError( "CModelDisplayPanel::CModelDisplayPanel: Null checkbox %u!\n", uiIndex );

		pCheckBoxSizer->Add( m_pCheckBoxes[ uiIndex ], wxGBPosition( uiIndex / NUM_CHECKBOXES_PER_ROW, ( uiIndex % NUM_CHECKBOXES_PER_ROW ) ), wxDefaultSpan, wxEXPAND );

		m_pCheckBoxes[ uiIndex ]->SetClientData( reinterpret_cast<void*>( uiIndex ) );
	}

	pCheckBoxPanel->SetSizer( pCheckBoxSizer );

	pControlsSizer->Add( pCheckBoxPanel, wxGBPosition( 0, 1 ), wxGBSpan( 4, 1 ), wxEXPAND );

	pControlsSizer->Add( m_pMeshScale, wxGBPosition( 0, 3 ), wxDefaultSpan, wxEXPAND );
	pControlsSizer->Add( m_pMeshScaleButton, wxGBPosition( 0, 4 ), wxDefaultSpan, wxEXPAND );

	pControlsSizer->Add( m_pBonesScale, wxGBPosition( 1, 3 ), wxDefaultSpan, wxEXPAND );
	pControlsSizer->Add( m_pBonesScaleButton, wxGBPosition( 1, 4 ), wxDefaultSpan, wxEXPAND );

	pControlsSizer->Add( m_pMirror[ 0 ], wxGBPosition( 0, 5 ), wxDefaultSpan, wxEXPAND );
	pControlsSizer->Add( m_pMirror[ 1 ], wxGBPosition( 1, 5 ), wxDefaultSpan, wxEXPAND );
	pControlsSizer->Add( m_pMirror[ 2 ], wxGBPosition( 2, 5 ), wxDefaultSpan, wxEXPAND );

	GetMainSizer()->Add( pControlsSizer );

	cvar::cvars().InstallGlobalCVarHandler( this );
}

CModelDisplayPanel::~CModelDisplayPanel()
{
	cvar::cvars().RemoveGlobalCVarHandler( this );
}

void CModelDisplayPanel::InitializeUI()
{
	SetRenderMode( RenderMode::TEXTURE_SHADED );
	SetOpacity( OPACITY_DEFAULT );

	m_pMeshScale->SetValue( "1.0" );
	m_pBonesScale->SetValue( "1.0" );

	auto pEntity = m_pHLMV->GetState()->GetEntity();

	for( size_t uiIndex = 0; uiIndex < 3; ++uiIndex )
	{
		m_pMirror[ uiIndex ]->SetValue( false );

		m_pMirror[ uiIndex ]->Enable( pEntity != nullptr );
	}
}

void CModelDisplayPanel::SetRenderMode( RenderMode renderMode )
{
	if( renderMode < RenderMode::FIRST )
		renderMode = RenderMode::FIRST;
	else if( renderMode > RenderMode::LAST )
		renderMode = RenderMode::LAST;

	m_pRenderMode->Select( static_cast<int>( renderMode ) );

	m_pHLMV->GetState()->renderMode = renderMode;
}

void CModelDisplayPanel::SetOpacity( int iValue, const bool bUpdateSlider )
{
	if( iValue < OPACITY_MIN )
		iValue = OPACITY_MIN;
	else if( iValue > OPACITY_MAX )
		iValue = OPACITY_MAX;

	m_pOpacity->SetLabelText( wxString::Format( "Opacity: %d%%", iValue ) );

	if( bUpdateSlider )
		m_pOpacitySlider->SetValue( iValue );

	auto pEntity = m_pHLMV->GetState()->GetEntity();

	if( !pEntity )
		return;

	pEntity->SetTransparency( iValue / static_cast<float>( OPACITY_MAX ) );
}

void CModelDisplayPanel::SetCheckBox( const CheckBox::Type checkBox, const bool bValue )
{
	InternalSetCheckBox( checkBox, bValue, false );
}

void CModelDisplayPanel::InternalSetCheckBox( const CheckBox::Type checkBox, const bool bValue, const bool bCameFromChangeEvent )
{
	if( checkBox < CheckBox::FIRST || checkBox > CheckBox::LAST )
		return;

	wxCheckBox* pCheckBox = m_pCheckBoxes[ checkBox ];

	//Don't do anything if it's identical. Helps prevent unnecessary calls.
	if( !bCameFromChangeEvent && pCheckBox->GetValue() == bValue )
		return;

	pCheckBox->SetValue( bValue );

	switch( checkBox )
	{
	case CheckBox::SHOW_HITBOXES:
		{
			cvar::cvars().Command( wxString::Format( "r_showhitboxes %d", bValue ? 1 : 0 ).c_str() );
			break;
		}

	case CheckBox::SHOW_GROUND:
		{
			m_pHLMV->GetState()->showGround = bValue;

			if( !m_pHLMV->GetState()->showGround && m_pHLMV->GetState()->mirror )
			{
				SetCheckBox( CheckBox::MIRROR_ON_GROUND, false );
			}

			break;
		}

	case CheckBox::SHOW_BONES:
		{
			cvar::cvars().Command( wxString::Format( "r_showbones %d", bValue ? 1 : 0 ).c_str() );

			break;
		}

	case CheckBox::MIRROR_ON_GROUND:
		{
			m_pHLMV->GetState()->mirror = bValue;

			if( m_pHLMV->GetState()->mirror && !m_pHLMV->GetState()->showGround )
			{
				SetCheckBox( CheckBox::SHOW_GROUND, true );
			}

			break;
		}

	case CheckBox::SHOW_ATTACHMENTS:
		{
			cvar::cvars().Command( wxString::Format( "r_showattachments %d", bValue ? 1 : 0 ).c_str() );
			break;
		}

	case CheckBox::SHOW_BACKGROUND:
		{
			m_pHLMV->GetState()->showBackground = bValue;
			break;
		}

	case CheckBox::SHOW_EYE_POSITION:
		{
			cvar::cvars().Command( wxString::Format( "r_showeyeposition %d", bValue ? 1 : 0 ).c_str() );
			break;
		}

	case CheckBox::WIREFRAME_OVERLAY:
		{
			m_pHLMV->GetState()->wireframeOverlay = bValue;
			break;
		}
	
	case CheckBox::BACKFACE_CULLING:
		{
			m_pHLMV->GetState()->backfaceCulling = bValue;
			break;
		}

	default: break;
	}
}

void CModelDisplayPanel::RenderModeChanged( wxCommandEvent& event )
{
	const int iValue = m_pRenderMode->GetSelection();

	if( iValue == wxNOT_FOUND )
		return;

	SetRenderMode( static_cast<RenderMode>( iValue ) );
}

void CModelDisplayPanel::OpacityChanged( wxCommandEvent& event )
{
	SetOpacity( m_pOpacitySlider->GetValue(), false );
}

void CModelDisplayPanel::CheckBoxChanged( wxCommandEvent& event )
{
	wxCheckBox* const pCheckBox = static_cast<wxCheckBox*>( event.GetEventObject() );

	const CheckBox::Type checkbox = static_cast<CheckBox::Type>( reinterpret_cast<int>( pCheckBox->GetClientData() ) );

	if( checkbox < CheckBox::FIRST || checkbox > CheckBox::LAST )
		return;

	InternalSetCheckBox( checkbox, pCheckBox->GetValue(), true );
}

void CModelDisplayPanel::ScaleMesh( wxCommandEvent& event )
{
	auto pEntity = m_pHLMV->GetState()->GetEntity();

	double flScale = 1.0;

	if( m_pMeshScale->GetValue().ToDouble( &flScale ) )
	{
		if( pEntity )
			studiomodel::ScaleMeshes( pEntity->GetModel(), flScale );
	}
	else
		m_pMeshScale->SetValue( "1.0" );
}

void CModelDisplayPanel::ScaleBones( wxCommandEvent& event )
{
	auto pEntity = m_pHLMV->GetState()->GetEntity();

	double flScale = 1.0;

	if( m_pBonesScale->GetValue().ToDouble( &flScale ) )
	{
		if( pEntity )
			studiomodel::ScaleBones( pEntity->GetModel(), flScale );
	}
	else
		m_pBonesScale->SetValue( "1.0" );
}

void CModelDisplayPanel::OnMirrorAxis( wxCommandEvent& event )
{
	auto pEntity = m_pHLMV->GetState()->GetEntity();

	if( !pEntity )
	{
		return;
	}

	const size_t uiIndex = *reinterpret_cast<const size_t*>( static_cast<wxCheckBox*>( event.GetEventObject() )->GetClientData() );

	pEntity->GetScale()[ uiIndex ] *= -1;
}

void CModelDisplayPanel::HandleCVar( cvar::CCVar& cvar, const char* pszOldValue, float flOldValue )
{
	if( strcmp( cvar.GetName(), "r_showbones" ) == 0 )
	{
		SetCheckBox( CheckBox::SHOW_BONES, cvar.GetBool() );
	}
	else if( strcmp( cvar.GetName(), "r_showattachments" ) == 0 )
	{
		SetCheckBox( CheckBox::SHOW_ATTACHMENTS, cvar.GetBool() );
	}
	else if( strcmp( cvar.GetName(), "r_showeyeposition" ) == 0 )
	{
		SetCheckBox( CheckBox::SHOW_EYE_POSITION, cvar.GetBool() );
	}
	else if( strcmp( cvar.GetName(), "r_showhitboxes" ) == 0 )
	{
		SetCheckBox( CheckBox::SHOW_HITBOXES, cvar.GetBool() );
	}
}
}