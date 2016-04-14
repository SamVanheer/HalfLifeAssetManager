#include <wx/gbsizer.h>

#include "hlmv/ui/CHLMV.h"
#include "hlmv/CHLMVState.h"

#include "studiomodel/StudioModel.h"

#include "CBodyPartsPanel.h"

wxBEGIN_EVENT_TABLE( CBodyPartsPanel, CBaseControlPanel )
	EVT_COMBOBOX( wxID_BODY_BODYPARTS, CBodyPartsPanel::BodypartChanged )
	EVT_COMBOBOX( wxID_BODY_SUBMODELS, CBodyPartsPanel::SubmodelChanged )
	EVT_COMBOBOX( wxID_BODY_SKIN, CBodyPartsPanel::SkinChanged )
	EVT_COMBOBOX( wxID_BODY_CONTROLLER, CBodyPartsPanel::ControllerChanged )
	EVT_SLIDER( wxID_BODY_CONTROLLER_SLIDER, CBodyPartsPanel::ControllerSliderChanged )
wxEND_EVENT_TABLE()

CBodyPartsPanel::CBodyPartsPanel( wxWindow* pParent, hlmv::CHLMV* const pHLMV )
	: CBaseControlPanel( pParent, "Body Parts", pHLMV )
{
	wxWindow* const pElemParent = GetBox();

	m_pBodypart = new wxComboBox( pElemParent, wxID_BODY_BODYPARTS, "", wxDefaultPosition, wxSize( COMBOBOX_WIDTH, wxDefaultSize.GetY() ) );
	m_pBodypart->SetEditable( false );

	wxStaticText* pBodypart = new wxStaticText( pElemParent, wxID_ANY, "Part" );

	m_pSubmodel = new wxComboBox( pElemParent, wxID_BODY_SUBMODELS, "", wxDefaultPosition, wxSize( COMBOBOX_WIDTH, wxDefaultSize.GetY() ) );
	m_pSubmodel->SetEditable( false );

	wxStaticText* pSubmodel = new wxStaticText( pElemParent, wxID_ANY, "Sub-model" );

	m_pSkin = new wxComboBox( pElemParent, wxID_BODY_SKIN, "", wxDefaultPosition, wxSize( COMBOBOX_WIDTH, wxDefaultSize.GetY() ) );
	m_pSkin->SetEditable( false );

	wxStaticText* pSkin = new wxStaticText( pElemParent, wxID_ANY, "Skin" );

	m_pController = new wxComboBox( pElemParent, wxID_BODY_CONTROLLER, "", wxDefaultPosition, wxSize( COMBOBOX_WIDTH, wxDefaultSize.GetY() ) );
	m_pController->SetEditable( false );

	wxStaticText* pController = new wxStaticText( pElemParent, wxID_ANY, "Controller" );

	m_pControllerSlider = new wxSlider( pElemParent, wxID_BODY_CONTROLLER_SLIDER, CONTROLLER_SLIDER_DEFAULT, CONTROLLER_SLIDER_MIN, CONTROLLER_SLIDER_MAX );

	wxStaticText* pControllerSlider = new wxStaticText( pElemParent, wxID_ANY, "Value" );

	m_pDrawnPolys = new wxStaticText( pElemParent, wxID_ANY, "Drawn Polys: Undefined" );

	m_pBones = new wxStaticText( pElemParent, wxID_ANY, "Bones: Undefined" );
	m_pBoneControllers = new wxStaticText( pElemParent, wxID_ANY, "Bone Controllers: Undefined" );
	m_pHitBoxes = new wxStaticText( pElemParent, wxID_ANY, "Hit Boxes: Undefined" );
	m_pSequences = new wxStaticText( pElemParent, wxID_ANY, "Sequences: Undefined" );
	m_pSequenceGroups = new wxStaticText( pElemParent, wxID_ANY, "Sequence Groups: Undefined" );

	m_pTextures = new wxStaticText( pElemParent, wxID_ANY, "Textures: Undefined" );
	m_pSkinFamilies = new wxStaticText( pElemParent, wxID_ANY, "Skin Families: Undefined" );
	m_pBodyparts = new wxStaticText( pElemParent, wxID_ANY, "Bodyparts: Undefined" );
	m_pAttachments = new wxStaticText( pElemParent, wxID_ANY, "Attachments: Undefined" );
	m_pTransitions = new wxStaticText( pElemParent, wxID_ANY, "Transitions: Undefined" );

	//Layout
	wxGridBagSizer* pSizer = new wxGridBagSizer( 5, 5 );

	pSizer->Add( m_pBodypart, wxGBPosition( 0, 0 ), wxDefaultSpan, wxCENTER );
	pSizer->Add( pBodypart, wxGBPosition( 0, 1 ), wxDefaultSpan, wxCENTER | wxALIGN_LEFT );

	pSizer->Add( m_pSubmodel, wxGBPosition( 1, 0 ), wxDefaultSpan, wxCENTER );
	pSizer->Add( pSubmodel, wxGBPosition( 1, 1 ), wxDefaultSpan, wxCENTER | wxALIGN_LEFT );

	pSizer->Add( m_pSkin, wxGBPosition( 2, 0 ), wxDefaultSpan, wxCENTER );
	pSizer->Add( pSkin, wxGBPosition( 2, 1 ), wxDefaultSpan, wxCENTER | wxALIGN_LEFT );

	pSizer->Add( m_pController, wxGBPosition( 0, 2 ), wxDefaultSpan, wxCENTER );
	pSizer->Add( pController, wxGBPosition( 0, 3 ), wxDefaultSpan, wxCENTER | wxALIGN_LEFT );

	pSizer->Add( m_pControllerSlider, wxGBPosition( 1, 2 ), wxDefaultSpan, wxCENTER );
	pSizer->Add( pControllerSlider, wxGBPosition( 1, 3 ), wxDefaultSpan, wxCENTER | wxALIGN_LEFT );

	pSizer->Add( m_pDrawnPolys, wxGBPosition( 2, 2 ), wxDefaultSpan, wxCENTER );

	//Info text
	wxGridBagSizer* pInfoSizer = new wxGridBagSizer( 5, 5 );

	pInfoSizer->Add( m_pBones, wxGBPosition( 0, 0 ), wxDefaultSpan, wxEXPAND );
	pInfoSizer->Add( m_pBoneControllers, wxGBPosition( 1, 0 ), wxDefaultSpan, wxEXPAND );
	pInfoSizer->Add( m_pHitBoxes, wxGBPosition( 2, 0 ), wxDefaultSpan, wxEXPAND );
	pInfoSizer->Add( m_pSequences, wxGBPosition( 3, 0 ), wxDefaultSpan, wxEXPAND );
	pInfoSizer->Add( m_pSequenceGroups, wxGBPosition( 4, 0 ), wxDefaultSpan, wxEXPAND );

	pInfoSizer->Add( m_pTextures, wxGBPosition( 0, 1 ), wxDefaultSpan, wxEXPAND );
	pInfoSizer->Add( m_pSkinFamilies, wxGBPosition( 1, 1 ), wxDefaultSpan, wxEXPAND );
	pInfoSizer->Add( m_pBodyparts, wxGBPosition( 2, 1 ), wxDefaultSpan, wxEXPAND );
	pInfoSizer->Add( m_pAttachments, wxGBPosition( 3, 1 ), wxDefaultSpan, wxEXPAND );
	pInfoSizer->Add( m_pTransitions, wxGBPosition( 4, 1 ), wxDefaultSpan, wxEXPAND );

	pSizer->Add( pInfoSizer, wxGBPosition( 0, 4 ), wxGBSpan( 2, 2 ), wxEXPAND | wxRESERVE_SPACE_EVEN_IF_HIDDEN );

	GetBoxSizer()->Add( pSizer );
}

CBodyPartsPanel::~CBodyPartsPanel()
{
}

void CBodyPartsPanel::ModelChanged( const StudioModel& model )
{
	const studiohdr_t* const pHdr = m_pHLMV->GetState()->GetStudioModel()->getStudioHeader();

	if( pHdr )
	{
		const mstudiobodyparts_t* const pbodyparts = ( mstudiobodyparts_t* ) ( ( byte* ) pHdr + pHdr->bodypartindex );

		m_pBodypart->Clear();

		wxArrayString names;

		if( pHdr->numbodyparts > 0 )
		{
			for( int i = 0; i < pHdr->numbodyparts; ++i )
				names.Add( pbodyparts[ i ].name );

			m_pBodypart->Append( names );

			m_pBodypart->Select( 0 );

			SetBodypart( 0 );

			names.Clear();
		}

		m_pSkin->Enable( pHdr->numskinfamilies > 0 );
		m_pSkin->Clear();

		for( int i = 0; i < pHdr->numskinfamilies; ++i )
		{
			names.Add( wxString::Format( "Skin %d", i + 1 ) );
		}

		m_pSkin->Append( names );

		SetSkin( 0 );

		names.Clear();

		m_pController->Enable( pHdr->numbonecontrollers > 0 );
		m_pControllerSlider->Enable( pHdr->numbonecontrollers > 0 );
		m_pController->Clear();

		const mstudiobonecontroller_t* const pbonecontrollers = ( mstudiobonecontroller_t* ) ( ( byte* ) pHdr + pHdr->bonecontrollerindex );

		for( int i = 0; i < pHdr->numbonecontrollers; i++ )
		{
			if( pbonecontrollers[ i ].index == CONTROLLER_MOUTH_INDEX )
			{
				names.Add( "Mouth" );
			}
			else
			{
				names.Add( wxString::Format( "Controller %d", pbonecontrollers[ i ].index ) );
			}
		}

		m_pController->Append( names );

		SetController( 0 );

		const studiohdr_t* const pTexHdr = model.getTextureHeader();

		m_pBones			->SetLabelText( wxString::Format( "Bones: %d", pHdr->numbones ) );
		m_pBoneControllers	->SetLabelText( wxString::Format( "Bone Controllers: %d", pHdr->numbonecontrollers ) );
		m_pHitBoxes			->SetLabelText( wxString::Format( "Hit Boxes: %d", pHdr->numhitboxes ) );
		m_pSequences		->SetLabelText( wxString::Format( "Sequences: %d", pHdr->numseq ) );
		m_pSequenceGroups	->SetLabelText( wxString::Format( "Sequence Groups: %d", pHdr->numseqgroups ) );

		m_pTextures			->SetLabelText( wxString::Format( "Textures: %d", pTexHdr->numtextures ) );
		m_pSkinFamilies		->SetLabelText( wxString::Format( "Skin Families: %d", pHdr->numskinfamilies ) );
		m_pBodyparts		->SetLabelText( wxString::Format( "Bodyparts: %d", pHdr->numbodyparts ) );
		m_pAttachments		->SetLabelText( wxString::Format( "Attachments: %d", pHdr->numattachments ) );
		m_pTransitions		->SetLabelText( wxString::Format( "Transitions: %d", pHdr->numtransitions ) );
	}
}

void CBodyPartsPanel::ViewUpdated()
{
	unsigned long uiOld = 0;

	m_pDrawnPolys->GetLabelText().ToULong( &uiOld );

	//Don't update if it's identical. Prevents flickering.
	if( uiOld != m_pHLMV->GetState()->drawnPolys )
		m_pDrawnPolys->SetLabelText( wxString::Format( "Drawn Polys: %u", m_pHLMV->GetState()->drawnPolys ) );
}

void CBodyPartsPanel::BodypartChanged( wxCommandEvent& event )
{
	SetBodypart( m_pBodypart->GetSelection() );
}

void CBodyPartsPanel::SubmodelChanged( wxCommandEvent& event )
{
	SetSubmodel( m_pSubmodel->GetSelection() );
}

void CBodyPartsPanel::SkinChanged( wxCommandEvent& event )
{
	SetSkin( m_pSkin->GetSelection() );
}

void CBodyPartsPanel::ControllerChanged( wxCommandEvent& event )
{
	SetController( m_pController->GetSelection() );
}

void CBodyPartsPanel::ControllerSliderChanged( wxCommandEvent& event )
{
	SetControllerValue( m_pController->GetSelection(), m_pControllerSlider->GetValue() );
}

void CBodyPartsPanel::SetBodypart( int iIndex )
{
	const studiohdr_t* const pHdr = m_pHLMV->GetState()->GetStudioModel()->getStudioHeader();

	if( !pHdr )
	{
		m_pBodypart->Select( 0 );
		return;
	}

	if( iIndex < 0 || iIndex >= pHdr->numbodyparts )
		iIndex = 0;

	m_pBodypart->Select( iIndex );

	if( iIndex < pHdr->numbodyparts )
	{
		const mstudiobodyparts_t* const pbodyparts = ( mstudiobodyparts_t* ) ( ( byte* ) pHdr + pHdr->bodypartindex );

		UpdateSubmodels( iIndex );
	}
}

void CBodyPartsPanel::SetSubmodel( int iIndex )
{
	m_pSubmodel->Select( iIndex );
	m_pHLMV->GetState()->GetStudioModel()->SetBodygroup( m_pBodypart->GetSelection(), iIndex );
}

void CBodyPartsPanel::SetSkin( int iIndex )
{
	m_pSkin->Select( iIndex );
	m_pHLMV->GetState()->GetStudioModel()->SetSkin( iIndex );
}

void CBodyPartsPanel::UpdateSubmodels( const int iIndex )
{
	m_pSubmodel->Clear();

	const studiohdr_t* const pHdr = m_pHLMV->GetState()->GetStudioModel()->getStudioHeader();

	if( !pHdr )
		return;

	if( iIndex < pHdr->numbodyparts )
	{
		const mstudiobodyparts_t* const pbodyparts = ( mstudiobodyparts_t* ) ( ( byte* ) pHdr + pHdr->bodypartindex );

		char szBuffer[ 64 ];

		for( int i = 0; i < pbodyparts[ iIndex ].nummodels; i++ )
		{
			snprintf( szBuffer, sizeof( szBuffer ), "Submodel %d", i + 1 );
			m_pSubmodel->Append( szBuffer );
		}

		m_pSubmodel->Select( 0 );
	}
}

void CBodyPartsPanel::SetController( int iIndex )
{
	const studiohdr_t* const pHdr = m_pHLMV->GetState()->GetStudioModel()->getStudioHeader();

	if( !pHdr )
		return;

	if( iIndex < 0 || iIndex >= pHdr->numbonecontrollers )
		iIndex = 0;

	const mstudiobonecontroller_t* const pbonecontrollers = ( mstudiobonecontroller_t* ) ( ( byte* ) pHdr + pHdr->bonecontrollerindex );

	m_pController->Select( iIndex );
	m_pControllerSlider->SetRange( ( int ) pbonecontrollers[ iIndex ].start, ( int ) pbonecontrollers[ iIndex ].end );
	m_pControllerSlider->SetValue( m_pHLMV->GetState()->GetStudioModel()->GetController( iIndex ) );
}

void CBodyPartsPanel::SetControllerValue( int iIndex, int iValue )
{
	const studiohdr_t* const pHdr = m_pHLMV->GetState()->GetStudioModel()->getStudioHeader();

	if( pHdr )
	{
		if( iIndex < 0 || iIndex >= pHdr->numbonecontrollers )
			return;

		const mstudiobonecontroller_t* const pbonecontrollers = ( mstudiobonecontroller_t* ) ( ( byte* ) pHdr + pHdr->bonecontrollerindex );

		if( pbonecontrollers[ iIndex ].index == CONTROLLER_MOUTH_INDEX )
			m_pHLMV->GetState()->GetStudioModel()->SetMouth( iValue );
		else
			m_pHLMV->GetState()->GetStudioModel()->SetController( pbonecontrollers[ iIndex ].index, iValue );
	}
}