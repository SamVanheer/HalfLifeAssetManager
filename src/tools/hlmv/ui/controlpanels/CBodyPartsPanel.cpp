#include <wx/gbsizer.h>

#include "../CModelViewerApp.h"
#include "../../CHLMVState.h"

#include "CBodyPartsPanel.h"

namespace hlmv
{
wxBEGIN_EVENT_TABLE( CBodyPartsPanel, CBaseControlPanel )
	EVT_CHOICE( wxID_BODY_BODYPARTS, CBodyPartsPanel::BodypartChanged )
	EVT_CHOICE( wxID_BODY_SUBMODELS, CBodyPartsPanel::SubmodelChanged )
	EVT_CHOICE( wxID_BODY_SKIN, CBodyPartsPanel::SkinChanged )
	EVT_CHOICE( wxID_BODY_CONTROLLER, CBodyPartsPanel::ControllerChanged )
	EVT_SLIDER( wxID_BODY_CONTROLLER_SLIDER, CBodyPartsPanel::ControllerSliderChanged )
wxEND_EVENT_TABLE()

CBodyPartsPanel::CBodyPartsPanel( wxWindow* pParent, CModelViewerApp* const pHLMV )
	: CBaseControlPanel( pParent, "Body Parts", pHLMV )
{
	wxWindow* const pElemParent = GetElementParent();

	m_pBodypart = new wxChoice( pElemParent, wxID_BODY_BODYPARTS, wxDefaultPosition, wxSize( COMBOBOX_WIDTH, wxDefaultSize.GetY() ) );

	wxStaticText* pBodypart = new wxStaticText( pElemParent, wxID_ANY, "Part" );

	m_pSubmodel = new wxChoice( pElemParent, wxID_BODY_SUBMODELS, wxDefaultPosition, wxSize( COMBOBOX_WIDTH, wxDefaultSize.GetY() ) );

	wxStaticText* pSubmodel = new wxStaticText( pElemParent, wxID_ANY, "Sub-model" );

	m_pSkin = new wxChoice( pElemParent, wxID_BODY_SKIN, wxDefaultPosition, wxSize( COMBOBOX_WIDTH, wxDefaultSize.GetY() ) );

	wxStaticText* pSkin = new wxStaticText( pElemParent, wxID_ANY, "Skin" );

	m_pController = new wxChoice( pElemParent, wxID_BODY_CONTROLLER, wxDefaultPosition, wxSize( COMBOBOX_WIDTH, wxDefaultSize.GetY() ) );

	wxStaticText* pController = new wxStaticText( pElemParent, wxID_ANY, "Controller" );

	m_pControllerSlider = new wxSlider( pElemParent, wxID_BODY_CONTROLLER_SLIDER, CONTROLLER_SLIDER_DEFAULT, CONTROLLER_SLIDER_MIN, CONTROLLER_SLIDER_MAX );

	wxStaticText* pControllerSlider = new wxStaticText( pElemParent, wxID_ANY, "Value" );

	m_pModelInfo = new wxPanel( pElemParent );

	m_pBones = new wxStaticText( m_pModelInfo, wxID_ANY, "Bones: Undefined" );
	m_pBoneControllers = new wxStaticText( m_pModelInfo, wxID_ANY, "Bone Controllers: Undefined" );
	m_pHitBoxes = new wxStaticText( m_pModelInfo, wxID_ANY, "Hit Boxes: Undefined" );
	m_pSequences = new wxStaticText( m_pModelInfo, wxID_ANY, "Sequences: Undefined" );
	m_pSequenceGroups = new wxStaticText( m_pModelInfo, wxID_ANY, "Sequence Groups: Undefined" );

	m_pTextures = new wxStaticText( m_pModelInfo, wxID_ANY, "Textures: Undefined" );
	m_pSkinFamilies = new wxStaticText( m_pModelInfo, wxID_ANY, "Skin Families: Undefined" );
	m_pBodyparts = new wxStaticText( m_pModelInfo, wxID_ANY, "Bodyparts: Undefined" );
	m_pAttachments = new wxStaticText( m_pModelInfo, wxID_ANY, "Attachments: Undefined" );
	m_pTransitions = new wxStaticText( m_pModelInfo, wxID_ANY, "Transitions: Undefined" );

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

	m_pModelInfo->SetSizer( pInfoSizer );

	pSizer->Add( m_pModelInfo, wxGBPosition( 0, 4 ), wxGBSpan( 3, 2 ), wxEXPAND | wxRESERVE_SPACE_EVEN_IF_HIDDEN );

	GetMainSizer()->Add( pSizer );
}

CBodyPartsPanel::~CBodyPartsPanel()
{
}

void CBodyPartsPanel::InitializeUI()
{
	m_pBodypart->Clear();
	m_pSubmodel->Clear();
	m_pSkin->Clear();
	m_pController->Clear();

	bool bSuccess = false;

	if( auto pEntity = m_pHLMV->GetState()->GetEntity() )
	{
		auto pModel = pEntity->GetModel();

		const studiohdr_t* const pHdr = pModel->GetStudioHeader();
		const auto pTextureHdr = pModel->GetTextureHeader();

		if( pHdr && pTextureHdr )
		{
			const mstudiobodyparts_t* const pbodyparts = ( mstudiobodyparts_t* ) ( ( byte* ) pHdr + pHdr->bodypartindex );

			wxArrayString names;

			if( pHdr->numbodyparts > 0 )
			{
				m_pBodypart->Enable( true );

				for( int i = 0; i < pHdr->numbodyparts; ++i )
					names.Add( pbodyparts[ i ].name );

				m_pBodypart->Append( names );

				m_pBodypart->Select( 0 );

				SetBodypart( 0 );

				names.Clear();
			}
			else
			{
				m_pBodypart->Enable( false );
				m_pSubmodel->Enable( false );
			}

			m_pSkin->Enable( pTextureHdr->numskinfamilies > 0 );

			for( int i = 0; i < pTextureHdr->numskinfamilies; ++i )
			{
				names.Add( wxString::Format( "Skin %d", i + 1 ) );
			}

			m_pSkin->Append( names );

			SetSkin( 0 );

			names.Clear();

			m_pController->Enable( pHdr->numbonecontrollers > 0 );
			m_pControllerSlider->Enable( pHdr->numbonecontrollers > 0 );

			const mstudiobonecontroller_t* const pbonecontrollers = ( mstudiobonecontroller_t* ) ( ( byte* ) pHdr + pHdr->bonecontrollerindex );

			for( int i = 0; i < pHdr->numbonecontrollers; i++ )
			{
				if( pbonecontrollers[ i ].index == STUDIO_MOUTH_CONTROLLER )
				{
					names.Add( "Mouth" );
				}
				else
				{
					names.Add( wxString::Format( "Controller %d", pbonecontrollers[ i ].index ) );
				}
			}

			m_pController->Append( names );

			const studiohdr_t* const pTexHdr = pModel->GetTextureHeader();

			SetModelInfo( *pHdr, *pTexHdr );

			m_pModelInfo->Show( true );

			bSuccess = true;
		}
	}

	if( !bSuccess )
	{
		m_pBodypart->Enable( false );
		m_pSubmodel->Enable( false );
		m_pSkin->Enable( false );
		m_pController->Enable( false );

		studiohdr_t dummyHdr;

		memset( &dummyHdr, 0, sizeof( dummyHdr ) );

		SetModelInfo( dummyHdr, dummyHdr );
		m_pModelInfo->Show( false );
	}

	SetController( 0 );
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
	auto pEntity = m_pHLMV->GetState()->GetEntity();

	if( !pEntity )
	{
		m_pBodypart->Select( 0 );
		return;
	}

	auto pModel = pEntity->GetModel();

	const auto pHdr = pModel->GetStudioHeader();

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
	auto pEntity = m_pHLMV->GetState()->GetEntity();

	if( !pEntity )
		return;

	m_pSubmodel->Select( iIndex );
	pEntity->SetBodygroup( m_pBodypart->GetSelection(), iIndex );
}

void CBodyPartsPanel::SetSkin( int iIndex )
{
	auto pEntity = m_pHLMV->GetState()->GetEntity();

	if( !pEntity )
		return;

	m_pSkin->Select( iIndex );
	pEntity->SetSkin( iIndex );
}

void CBodyPartsPanel::UpdateSubmodels( const int iIndex )
{
	auto pEntity = m_pHLMV->GetState()->GetEntity();

	m_pSubmodel->Clear();

	if( !pEntity )
	{
		m_pSubmodel->Enable( false );
		return;
	}

	m_pSubmodel->Enable( true );

	auto pHdr = pEntity->GetModel()->GetStudioHeader();

	if( iIndex < pHdr->numbodyparts )
	{
		const mstudiobodyparts_t* const pbodyparts = ( mstudiobodyparts_t* ) ( ( byte* ) pHdr + pHdr->bodypartindex );

		char szBuffer[ 64 ];

		for( int i = 0; i < pbodyparts[ iIndex ].nummodels; i++ )
		{
			if( !PrintfSuccess( snprintf( szBuffer, sizeof( szBuffer ), "Submodel %d", i + 1 ), sizeof( szBuffer ) ) )
				szBuffer[ 0 ] = '\0';

			m_pSubmodel->Append( szBuffer );
		}

		m_pSubmodel->Select( 0 );
	}
}

void CBodyPartsPanel::SetModelInfo( const studiohdr_t& hdr, const studiohdr_t& texHdr )
{
	m_pBones			->SetLabelText( wxString::Format( "Bones: %d", hdr.numbones ) );
	m_pBoneControllers	->SetLabelText( wxString::Format( "Bone Controllers: %d", hdr.numbonecontrollers ) );
	m_pHitBoxes			->SetLabelText( wxString::Format( "Hit Boxes: %d", hdr.numhitboxes ) );
	m_pSequences		->SetLabelText( wxString::Format( "Sequences: %d", hdr.numseq ) );
	m_pSequenceGroups	->SetLabelText( wxString::Format( "Sequence Groups: %d", hdr.numseqgroups ) );

	m_pTextures			->SetLabelText( wxString::Format( "Textures: %d", texHdr.numtextures ) );
	m_pSkinFamilies		->SetLabelText( wxString::Format( "Skin Families: %d", texHdr.numskinfamilies ) );
	m_pBodyparts		->SetLabelText( wxString::Format( "Bodyparts: %d", hdr.numbodyparts ) );
	m_pAttachments		->SetLabelText( wxString::Format( "Attachments: %d", hdr.numattachments ) );
	m_pTransitions		->SetLabelText( wxString::Format( "Transitions: %d", hdr.numtransitions ) );
}

void CBodyPartsPanel::SetController( int iIndex )
{
	auto pEntity = m_pHLMV->GetState()->GetEntity();

	if( !pEntity || !pEntity->GetModel() || !pEntity->GetModel()->GetStudioHeader() || pEntity->GetModel()->GetStudioHeader()->numbonecontrollers == 0 )
	{
		//Disable and center it.
		m_pControllerSlider->Enable( false );
		m_pControllerSlider->SetRange( 0, 2 );
		m_pControllerSlider->SetValue( 1 );
		return;
	}

	const studiohdr_t* const pHdr = pEntity->GetModel()->GetStudioHeader();

	if( iIndex < 0 || iIndex >= pHdr->numbonecontrollers )
		iIndex = 0;

	const mstudiobonecontroller_t* const pbonecontrollers = ( mstudiobonecontroller_t* ) ( ( byte* ) pHdr + pHdr->bonecontrollerindex );

	const mstudiobonecontroller_t& controller = pbonecontrollers[ iIndex ];

	m_pController->Select( iIndex );

	float flStart, flEnd;

	//Swap values if the range is inverted.
	if( controller.end < controller.start )
	{
		flStart = controller.end;
		flEnd = controller.start;
	}
	else
	{
		flStart = controller.start;
		flEnd = controller.end;
	}

	//Should probably scale as needed so the range is sufficiently large.
	//This prevents ranges that cover less than a whole integer from not doing anything.
	if( ( flEnd - flStart ) < 1.0f )
	{
		m_flControllerSliderScale = 100.0f;
	}
	else
	{
		m_flControllerSliderScale = 1.0f;
	}

	m_pControllerSlider->SetRange( ( int ) ( flStart * m_flControllerSliderScale ), ( int ) ( flEnd * m_flControllerSliderScale ) );

	m_pControllerSlider->SetValue( pEntity->GetControllerValue( iIndex ) );
	m_pControllerSlider->Enable( true );
}

void CBodyPartsPanel::SetControllerValue( int iIndex, int iValue )
{
	auto pEntity = m_pHLMV->GetState()->GetEntity();

	if( !pEntity )
		return;

	const studiohdr_t* const pHdr = pEntity->GetModel()->GetStudioHeader();

	if( pHdr )
	{
		if( iIndex < 0 || iIndex >= pHdr->numbonecontrollers )
			return;

		const mstudiobonecontroller_t* const pbonecontrollers = ( mstudiobonecontroller_t* ) ( ( byte* ) pHdr + pHdr->bonecontrollerindex );

		const float flValue = ( static_cast<float>( iValue ) / m_flControllerSliderScale );

		//TODO: support multiple mouth controllers somehow.
		if( pbonecontrollers[ iIndex ].index == STUDIO_MOUTH_CONTROLLER )
			pEntity->SetMouth( flValue );
		else
			pEntity->SetController( pbonecontrollers[ iIndex ].index, flValue );
	}
}
}