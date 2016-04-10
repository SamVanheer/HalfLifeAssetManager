#include <wx/gbsizer.h>

#include "model/studiomodel/StudioModel.h"

#include "CModelDisplayPanel.h"

static const wxString RENDERMODES[] =
{
	"Wireframe",
	"Flat Shaded",
	"Smooth Shaded",
	"Texture Shaded"
};

static const size_t DEFAULT_RENDERMODE = ARRAYSIZE( RENDERMODES ) - 1;

wxBEGIN_EVENT_TABLE( CModelDisplayPanel, CBaseControlPanel )
	EVT_COMBOBOX( wxID_MDLDISP_RENDERMODE, CModelDisplayPanel::RenderModeChanged )
	EVT_SLIDER( wxID_MDLDISP_OPACITY, CModelDisplayPanel::OpacityChanged )
	EVT_CHECKBOX( wxID_MDLDISP_CHECKBOX, CModelDisplayPanel::CheckBoxChanged )
	EVT_BUTTON( wxID_MDLDISP_SCALEMESH, CModelDisplayPanel::ScaleMesh )
	EVT_BUTTON( wxID_MDLDISP_SCALEBONES, CModelDisplayPanel::ScaleBones )
wxEND_EVENT_TABLE()

CModelDisplayPanel::CModelDisplayPanel( wxWindow* pParent, CHLMVSettings* const pSettings )
	: CBaseControlPanel( pParent, "Model Display", pSettings )
{
	//Helps catch errors if we miss one.
	memset( m_pCheckBoxes, 0, sizeof( m_pCheckBoxes ) );

	wxWindow* const pElemParent = GetBox();

	wxStaticText* pRenderMode = new wxStaticText( pElemParent, wxID_ANY, "Render mode:" );

	m_pOpacity = new wxStaticText( pElemParent, wxID_ANY, "Opacity: Undefined%" );

	m_pOpacitySlider = new wxSlider( pElemParent, wxID_MDLDISP_OPACITY, OPACITY_DEFAULT, OPACITY_MIN, OPACITY_MAX );

	m_pRenderMode = new wxComboBox( pElemParent, wxID_MDLDISP_RENDERMODE, RENDERMODES[ DEFAULT_RENDERMODE ], wxDefaultPosition, wxDefaultSize, ARRAYSIZE( RENDERMODES ), RENDERMODES );

	m_pRenderMode->SetEditable( false );

	m_pCheckBoxes[ CheckBox::SHOW_HITBOXES ]		= new wxCheckBox( pElemParent, wxID_MDLDISP_CHECKBOX, "Show Hit Boxes" );
	m_pCheckBoxes[ CheckBox::SHOW_GROUND ]			= new wxCheckBox( pElemParent, wxID_MDLDISP_CHECKBOX, "Show Ground" );
	m_pCheckBoxes[ CheckBox::SHOW_BONES ]			= new wxCheckBox( pElemParent, wxID_MDLDISP_CHECKBOX, "Show Bones" );
	m_pCheckBoxes[ CheckBox::MIRROR_ON_GROUND ]		= new wxCheckBox( pElemParent, wxID_MDLDISP_CHECKBOX, "Mirror Model On Ground" );
	m_pCheckBoxes[ CheckBox::SHOW_ATTACHMENTS ]		= new wxCheckBox( pElemParent, wxID_MDLDISP_CHECKBOX, "Show Attachments" );
	m_pCheckBoxes[ CheckBox::SHOW_BACKGROUND ]		= new wxCheckBox( pElemParent, wxID_MDLDISP_CHECKBOX, "Show Background" );
	m_pCheckBoxes[ CheckBox::SHOW_EYE_POSITION ]	= new wxCheckBox( pElemParent, wxID_MDLDISP_CHECKBOX, "Show Eye Position" );
	m_pCheckBoxes[ CheckBox::WIREFRAME_OVERLAY ]	= new wxCheckBox( pElemParent, wxID_MDLDISP_CHECKBOX, "Wireframe Overlay" );

	m_pMeshScale = new wxTextCtrl( pElemParent, wxID_ANY, "1.0" );
	m_pMeshScaleButton = new wxButton( pElemParent, wxID_MDLDISP_SCALEMESH, "Scale Mesh" );

	m_pBonesScale = new wxTextCtrl( pElemParent, wxID_ANY, "1.0" );
	m_pBonesScaleButton = new wxButton( pElemParent, wxID_MDLDISP_SCALEBONES, "Scale Bones" );

	m_pDrawnPolys = new wxStaticText( pElemParent, wxID_ANY, "Drawn Polys: Undefined" );

	//Layout
	wxGridBagSizer* pControlsSizer = new wxGridBagSizer( CONTROLS_ROW_GAP, CONTROLS_COL_GAP );

	pControlsSizer->Add( pRenderMode, wxGBPosition( 0, 0 ), wxDefaultSpan, wxEXPAND );
	pControlsSizer->Add( m_pRenderMode, wxGBPosition( 1, 0 ), wxDefaultSpan, wxEXPAND );

	pControlsSizer->Add( m_pOpacity, wxGBPosition( 2, 0 ), wxDefaultSpan, wxEXPAND );
	pControlsSizer->Add( m_pOpacitySlider, wxGBPosition( 3, 0 ), wxDefaultSpan, wxEXPAND );

	for( size_t uiIndex = CheckBox::FIRST; uiIndex < CheckBox::COUNT; ++uiIndex )
	{
		if( !m_pCheckBoxes[ uiIndex ] )
			wxLogError( "CModelDisplayPanel::CModelDisplayPanel: Null checkbox %u!\n", uiIndex );

		pControlsSizer->Add( m_pCheckBoxes[ uiIndex ], wxGBPosition( uiIndex / NUM_CHECKBOXES_PER_ROW, 1 + ( uiIndex % NUM_CHECKBOXES_PER_ROW ) ), wxDefaultSpan, wxEXPAND );

		m_pCheckBoxes[ uiIndex ]->SetClientData( reinterpret_cast<void*>( uiIndex ) );
	}

	pControlsSizer->Add( m_pMeshScale, wxGBPosition( 0, 3 ), wxDefaultSpan, wxEXPAND );
	pControlsSizer->Add( m_pMeshScaleButton, wxGBPosition( 0, 4 ), wxDefaultSpan, wxEXPAND );

	pControlsSizer->Add( m_pBonesScale, wxGBPosition( 1, 3 ), wxDefaultSpan, wxEXPAND );
	pControlsSizer->Add( m_pBonesScaleButton, wxGBPosition( 1, 4 ), wxDefaultSpan, wxEXPAND );

	pControlsSizer->Add( m_pDrawnPolys, wxGBPosition( 3, 3 ), wxGBSpan( 1, 2 ), wxEXPAND );

	GetBoxSizer()->Add( pControlsSizer );
}

CModelDisplayPanel::~CModelDisplayPanel()
{
}

void CModelDisplayPanel::ModelChanged( const StudioModel& model )
{
	SetRenderMode( RenderMode::TEXTURE_SHADED );
	SetOpacity( 100 );

	m_pMeshScale->SetValue( "1.0" );
	m_pBonesScale->SetValue( "1.0" );
}

void CModelDisplayPanel::ViewUpdated()
{
	//Don't update if it's identical. Prevents flickering.
	if( m_uiDrawnPolysLast != m_pSettings->drawnPolys )
	{
		m_uiDrawnPolysLast = m_pSettings->drawnPolys;
		m_pDrawnPolys->SetLabelText( wxString::Format( "Drawn Polys: %u", m_pSettings->drawnPolys ) );
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

	switch( checkbox )
	{
	case CheckBox::SHOW_HITBOXES:
		{
			m_pSettings->renderSettings.showHitBoxes = pCheckBox->GetValue();
			break;
		}

	case CheckBox::SHOW_GROUND:
		{
			m_pSettings->showGround = pCheckBox->GetValue();

			//TODO: handle checkbox setting somewhere else
			if( !m_pSettings->showGround && m_pSettings->mirror )
			{
				m_pCheckBoxes[ CheckBox::MIRROR_ON_GROUND ]->SetValue( false );
				m_pSettings->mirror = false;
			}

			break;
		}

	case CheckBox::SHOW_BONES:
		{
			m_pSettings->renderSettings.showBones = pCheckBox->GetValue();
			break;
		}

	case CheckBox::MIRROR_ON_GROUND:
		{
			m_pSettings->mirror = pCheckBox->GetValue();

			//TODO: handle checkbox setting somewhere else
			if( m_pSettings->mirror && !m_pSettings->showGround )
			{
				m_pCheckBoxes[ CheckBox::SHOW_GROUND ]->SetValue( true );
				m_pSettings->showGround = true;
			}

			break;
		}

	case CheckBox::SHOW_ATTACHMENTS:
		{
			m_pSettings->renderSettings.showAttachments = pCheckBox->GetValue();
			break;
		}

	case CheckBox::SHOW_BACKGROUND:
		{
			m_pSettings->showBackground = pCheckBox->GetValue();
			break;
		}

	case CheckBox::SHOW_EYE_POSITION:
		{
			m_pSettings->renderSettings.showEyePosition = pCheckBox->GetValue();
			break;
		}

	case CheckBox::WIREFRAME_OVERLAY:
		{
			m_pSettings->wireframeOverlay = pCheckBox->GetValue();
			break;
		}

	default: break;
	}
}

void CModelDisplayPanel::ScaleMesh( wxCommandEvent& event )
{
	double flScale = 1.0;

	if( m_pMeshScale->GetValue().ToDouble( &flScale ) )
		m_pSettings->GetStudioModel()->scaleMeshes( flScale );
	else
		m_pMeshScale->SetValue( "1.0" );
}

void CModelDisplayPanel::ScaleBones( wxCommandEvent& event )
{
	double flScale = 1.0;

	if( m_pBonesScale->GetValue().ToDouble( &flScale ) )
		m_pSettings->GetStudioModel()->scaleBones( flScale );
	else
		m_pBonesScale->SetValue( "1.0" );
}

void CModelDisplayPanel::SetRenderMode( RenderMode renderMode )
{
	if( renderMode < RenderMode::FIRST )
		renderMode = RenderMode::FIRST;
	else if( renderMode > RenderMode::LAST )
		renderMode = RenderMode::LAST;

	m_pRenderMode->Select( static_cast<int>( renderMode ) );

	m_pSettings->renderMode = renderMode;
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

	m_pSettings->renderSettings.transparency = iValue / static_cast<float>( OPACITY_MAX );
}