#include <wx/gbsizer.h>
#include <wx/statline.h>

#include "tools/shared/ui/CStudioTypesCheatSheet.h"

#include "../CHLMV.h"
#include "../../CHLMVState.h"

#include "CBonesPanel.h"

namespace hlmv
{
wxBEGIN_EVENT_TABLE( CBonesPanel, CBaseControlPanel )
	EVT_COMBOBOX( wxID_BONES_BONES, CBonesPanel::OnBoneChanged )
	EVT_BUTTON( wxID_BONES_SHOWBONECONTROLLERINFO, CBonesPanel::OnShowBoneController )
	EVT_COMBOBOX( wxID_BONES_BONECONTROLLERS, CBonesPanel::OnBoneControllerChanged )
	EVT_BUTTON( wxID_BONES_SHOWBONEINFO, CBonesPanel::OnShowBone )
	EVT_BUTTON( wxID_BONES_SHOWTYPESCHEATSHEET, CBonesPanel::OnShowTypesCheatSheet )
wxEND_EVENT_TABLE()

CBonesPanel::CBonesPanel( wxWindow* pParent, CHLMV* const pHLMV )
	: CBaseControlPanel( pParent, "Bones", pHLMV )
{
	auto pElemParent = GetElementParent();

	auto pBoneText = new wxStaticText( pElemParent, wxID_ANY, "Bone:" );

	m_pBones = new wxComboBox( pElemParent, wxID_BONES_BONES, wxEmptyString, wxDefaultPosition, wxSize( 200, wxDefaultSize.GetHeight() ) );
	m_pBones->SetEditable( false );

	m_pBoneInfo = new wxPanel( pElemParent, wxID_ANY, wxDefaultPosition, wxSize( 425, wxDefaultSize.GetHeight() ) );

	m_pBoneName			= new wxStaticText( m_pBoneInfo, wxID_ANY, "Bone Name: Undefined", wxDefaultPosition, wxSize( 200, wxDefaultSize.GetHeight() ) );
	m_pParentBone		= new wxStaticText( m_pBoneInfo, wxID_ANY, "Parent Bone: Undefined", wxDefaultPosition, wxSize( 200, wxDefaultSize.GetHeight() ) );
	m_pBoneFlags		= new wxStaticText( m_pBoneInfo, wxID_ANY, "Bone Flags: Undefined", wxDefaultPosition, wxSize( 200, wxDefaultSize.GetHeight() ) );
	m_Position			= new wxStaticText( m_pBoneInfo, wxID_ANY, "Position: Undefined", wxDefaultPosition, wxSize( 200, wxDefaultSize.GetHeight() ) );
	m_pPositionScale	= new wxStaticText( m_pBoneInfo, wxID_ANY, "Position Scale: Undefined", wxDefaultPosition, wxSize( 200, wxDefaultSize.GetHeight() ) );
	m_pRotation			= new wxStaticText( m_pBoneInfo, wxID_ANY, "Rotation: Undefined", wxDefaultPosition, wxSize( 200, wxDefaultSize.GetHeight() ) );
	m_pRotationScale	= new wxStaticText( m_pBoneInfo, wxID_ANY, "Rotation Scale: Undefined", wxDefaultPosition, wxSize( 200, wxDefaultSize.GetHeight() ) );

	auto pBoneBoneControllersText = new wxStaticText( m_pBoneInfo, wxID_ANY, "Bone Controller (Current Bone):" );

	m_pBoneBoneControllers = new wxComboBox( m_pBoneInfo, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( 200, wxDefaultSize.GetHeight() ) );
	m_pBoneBoneControllers->SetEditable( false );

	m_pShowBoneControllerInfo = new wxButton( m_pBoneInfo, wxID_BONES_SHOWBONECONTROLLERINFO, ">>" );

	auto pBoneControllersText = new wxStaticText( pElemParent, wxID_ANY, "Bone Controller:" );

	m_pBoneControllers = new wxComboBox( pElemParent, wxID_BONES_BONECONTROLLERS, wxEmptyString, wxDefaultPosition, wxSize( 200, wxDefaultSize.GetHeight() ) );
	m_pBoneControllers->SetEditable( false );

	m_pShowBoneInfo = new wxButton( pElemParent, wxID_BONES_SHOWBONEINFO, "<<" );

	m_pBoneControllerInfo = new wxPanel( pElemParent, wxID_ANY, wxDefaultPosition, wxSize( 425, wxDefaultSize.GetHeight() ) );

	m_pControllerBone	= new wxStaticText( m_pBoneControllerInfo, wxID_ANY, "Bone: Undefined", wxDefaultPosition, wxSize( 200, wxDefaultSize.GetHeight() ) );
	m_pType				= new wxStaticText( m_pBoneControllerInfo, wxID_ANY, "Type: Undefined", wxDefaultPosition, wxSize( 200, wxDefaultSize.GetHeight() ) );
	m_pStart			= new wxStaticText( m_pBoneControllerInfo, wxID_ANY, "Start: Undefined", wxDefaultPosition, wxSize( 200, wxDefaultSize.GetHeight() ) );
	m_pEnd				= new wxStaticText( m_pBoneControllerInfo, wxID_ANY, "End: Undefined", wxDefaultPosition, wxSize( 200, wxDefaultSize.GetHeight() ) );
	m_pRest				= new wxStaticText( m_pBoneControllerInfo, wxID_ANY, "Rest: Undefined", wxDefaultPosition, wxSize( 200, wxDefaultSize.GetHeight() ) );
	m_pIndex			= new wxStaticText( m_pBoneControllerInfo, wxID_ANY, "Index: Undefined", wxDefaultPosition, wxSize( 200, wxDefaultSize.GetHeight() ) );

	auto pTypesText = new wxStaticText( m_pBoneControllerInfo, wxID_ANY, "Types:" );
	m_pTypes = new wxListBox( m_pBoneControllerInfo, wxID_ANY, wxDefaultPosition, wxSize( 200, wxDefaultSize.GetHeight() ) );

	m_pTypesCheatSheet = new wxButton( pElemParent, wxID_BONES_SHOWTYPESCHEATSHEET, "Types Cheat Sheet" );

	//Layout
	auto pSizer = new wxGridBagSizer( 5, 5 );

	int iCol = 0;

	pSizer->Add( pBoneText, wxGBPosition( 0, iCol ), wxGBSpan( 1, 1 ), wxEXPAND );
	pSizer->Add( m_pBones, wxGBPosition( 1, iCol++ ), wxGBSpan( 1, 1 ), wxEXPAND );

	auto pBoneInfoSizer = new wxGridBagSizer( 5, 5 );

	pBoneInfoSizer->Add( m_pBoneName, wxGBPosition( 0, 0 ), wxGBSpan( 1, 1 ), wxEXPAND );
	pBoneInfoSizer->Add( m_pParentBone, wxGBPosition( 1, 0 ), wxGBSpan( 1, 1 ), wxEXPAND );
	pBoneInfoSizer->Add( m_pBoneFlags, wxGBPosition( 2, 0 ), wxGBSpan( 1, 1 ), wxEXPAND );
	pBoneInfoSizer->Add( m_Position, wxGBPosition( 3, 0 ), wxGBSpan( 1, 1 ), wxEXPAND );
	pBoneInfoSizer->Add( m_pPositionScale, wxGBPosition( 4, 0 ), wxGBSpan( 1, 1 ), wxEXPAND );
	pBoneInfoSizer->Add( m_pRotation, wxGBPosition( 5, 0 ), wxGBSpan( 1, 1 ), wxEXPAND );
	pBoneInfoSizer->Add( m_pRotationScale, wxGBPosition( 6, 0 ), wxGBSpan( 1, 1 ), wxEXPAND );

	pBoneInfoSizer->Add( pBoneBoneControllersText, wxGBPosition( 0, 1 ), wxGBSpan( 1, 2 ), wxEXPAND );
	pBoneInfoSizer->Add( m_pBoneBoneControllers, wxGBPosition( 1, 1 ), wxGBSpan( 1, 2 ), wxEXPAND );
	pBoneInfoSizer->Add( m_pShowBoneControllerInfo, wxGBPosition( 2, 2 ), wxGBSpan( 1, 1 ), wxEXPAND );

	m_pBoneInfo->SetSizer( pBoneInfoSizer );

	pSizer->Add( m_pBoneInfo, wxGBPosition( 0, iCol++ ), wxGBSpan( 4, 1 ), wxEXPAND | wxRESERVE_SPACE_EVEN_IF_HIDDEN );

	pSizer->Add( new wxStaticLine( pElemParent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_VERTICAL ), wxGBPosition( 0, iCol++ ), wxGBSpan( 4, 1 ), wxEXPAND );

	pSizer->Add( pBoneControllersText, wxGBPosition( 0, iCol ), wxGBSpan( 1, 2 ), wxEXPAND );
	pSizer->Add( m_pBoneControllers, wxGBPosition( 1, iCol ), wxGBSpan( 1, 2 ), wxEXPAND );
	pSizer->Add( m_pShowBoneInfo, wxGBPosition( 2, iCol ), wxGBSpan( 1, 1 ), wxEXPAND );

	iCol += 2;

	auto pInfoSizer = new wxGridBagSizer( 5, 5 );

	pInfoSizer->Add( m_pControllerBone, wxGBPosition( 0, 0 ), wxGBSpan( 1, 1 ), wxEXPAND );
	pInfoSizer->Add( m_pType, wxGBPosition( 1, 0 ), wxGBSpan( 1, 1 ), wxEXPAND );
	pInfoSizer->Add( m_pStart, wxGBPosition( 2, 0 ), wxGBSpan( 1, 1 ), wxEXPAND );
	pInfoSizer->Add( m_pEnd, wxGBPosition( 3, 0 ), wxGBSpan( 1, 1 ), wxEXPAND );
	pInfoSizer->Add( m_pRest, wxGBPosition( 4, 0 ), wxGBSpan( 1, 1 ), wxEXPAND );
	pInfoSizer->Add( m_pIndex, wxGBPosition( 5, 0 ), wxGBSpan( 1, 1 ), wxEXPAND );

	pInfoSizer->Add( pTypesText, wxGBPosition( 0, 1 ), wxGBSpan( 1, 1 ), wxEXPAND );
	pInfoSizer->Add( m_pTypes, wxGBPosition( 1, 1 ), wxGBSpan( 5, 1 ), wxEXPAND );

	m_pBoneControllerInfo->SetSizer( pInfoSizer );

	pSizer->Add( m_pBoneControllerInfo, wxGBPosition( 0, iCol ), wxGBSpan( 4, 2 ), wxEXPAND | wxRESERVE_SPACE_EVEN_IF_HIDDEN );

	iCol += 2;

	pSizer->Add( m_pTypesCheatSheet, wxGBPosition( 0, iCol++ ), wxGBSpan( 1, 1 ), wxEXPAND );

	GetMainSizer()->Add( pSizer );
}

CBonesPanel::~CBonesPanel()
{
}

void CBonesPanel::InitializeUI()
{
	m_pBones->Clear();
	m_pBoneControllers->Clear();

	auto pEntity = m_pHLMV->GetState()->GetEntity();

	bool bHasBones = false;

	bool bHasBoneControllers = false;

	if( pEntity )
	{
		auto pModel = pEntity->GetModel();

		auto pStudioHdr = pModel->GetStudioHeader();

		wxArrayString items;

		for( int iIndex = 0; iIndex < pStudioHdr->numbones; ++iIndex )
		{
			auto pBone = pStudioHdr->GetBone( iIndex );

			items.push_back( wxString::Format( "Bone %d = %s", iIndex, pBone->name ) );
		}

		m_pBones->Append( items );

		bHasBones = pStudioHdr->numbones > 0;

		items.clear();

		for( int iIndex = 0; iIndex < pStudioHdr->numbonecontrollers; ++iIndex )
		{
			auto pBoneController = pStudioHdr->GetBoneController( iIndex );

			if( pBoneController->index != STUDIO_MOUTH_CONTROLLER )
			{
				items.push_back( wxString::Format( "Bone Controller %d", iIndex ) );
			}
			else
			{
				items.push_back( wxString::Format( "Bone Controller %d (Mouth)", iIndex ) );
			}
		}

		m_pBoneControllers->Append( items );

		bHasBoneControllers = pStudioHdr->numbonecontrollers > 0;
	}

	m_pBones->Enable( bHasBones );
	m_pBoneControllers->Enable( bHasBoneControllers );
	m_pShowBoneInfo->Enable( bHasBoneControllers );

	SetBone( 0 );
	SetBoneController( 0 );
}

void CBonesPanel::SetBone( int iIndex )
{
	auto pEntity = m_pHLMV->GetState()->GetEntity();

	if( !pEntity )
	{
		m_pBoneInfo->Show( false );
		return;
	}

	auto pModel = pEntity->GetModel();

	auto pStudioHdr = pModel->GetStudioHeader();

	if( pStudioHdr->numbones <= 0 )
	{
		m_pBoneInfo->Show( false );
		return;
	}

	if( iIndex < 0 || iIndex >= pStudioHdr->numbones )
		iIndex = 0;

	auto pBone = pStudioHdr->GetBone( iIndex );

	m_pBoneName->SetLabelText( wxString::Format( "Bone Name: %s", pBone->name ) );

	if( pBone->parent != -1 )
	{
		m_pParentBone->SetLabelText( wxString::Format( "Parent Bone: %s (%d)", pStudioHdr->GetBone( pBone->parent )->name, pBone->parent ) );
	}
	else
	{
		m_pParentBone->SetLabelText( wxString::Format( "Parent Bone: (None) (-1)" ) );
	}

	m_pBoneFlags->SetLabelText( wxString::Format( "Bone Flags: %d", pBone->flags ) );
	m_Position->SetLabelText( wxString::Format( "Position: %.2f %.2f %.2f", pBone->value[ 0 ], pBone->value[ 1 ], pBone->value[ 2 ] ) );
	m_pPositionScale->SetLabelText( wxString::Format( "Position Scale: %.2f %.2f %.2f", pBone->scale[ 0 ], pBone->scale[ 1 ], pBone->scale[ 2 ] ) );
	m_pRotation->SetLabelText( wxString::Format( "Rotation: %.2f %.2f %.2f", pBone->value[ 3 ], pBone->value[ 4 ], pBone->value[ 5 ] ) );
	m_pRotationScale->SetLabelText( wxString::Format( "Rotation Scale: %.2f %.2f %.2f", pBone->scale[ 3 ], pBone->scale[ 4 ], pBone->scale[ 5 ] ) );

	m_pBoneBoneControllers->Clear();

	wxArrayString items;

	//TODO: magic number
	for( int iController = 0; iController < 6; ++iController )
	{
		if( pBone->bonecontroller[ iController ] != -1 )
			items.push_back( wxString::Format( "Controller %d = %d", iController, pBone->bonecontroller[ iController ] ) );
	}

	m_pBoneBoneControllers->Append( items );

	m_pBoneBoneControllers->Select( 0 );

	m_pBoneBoneControllers->Enable( !items.empty() );

	m_pShowBoneControllerInfo->Enable( !items.empty() );

	m_pBones->Select( iIndex );

	m_pBoneInfo->Show( true );
}

void CBonesPanel::SetBoneController( int iIndex )
{
	auto pEntity = m_pHLMV->GetState()->GetEntity();

	if( !pEntity )
	{
		m_pBoneControllerInfo->Show( false );
		return;
	}

	auto pModel = pEntity->GetModel();

	auto pStudioHdr = pModel->GetStudioHeader();

	if( pStudioHdr->numbonecontrollers <= 0 )
	{
		m_pBoneControllerInfo->Show( false );
		return;
	}

	if( iIndex < 0 || iIndex >= pStudioHdr->numbonecontrollers )
		iIndex = 0;

	auto pBoneController = pStudioHdr->GetBoneController( iIndex );

	m_pControllerBone->SetLabelText( wxString::Format( "Bone: %d", pBoneController->bone ) );
	m_pType->SetLabelText( wxString::Format( "Type: %d", pBoneController->type ) );
	m_pStart->SetLabelText( wxString::Format( "Start: %f", pBoneController->start ) );
	m_pEnd->SetLabelText( wxString::Format( "End: %f", pBoneController->end ) );
	m_pRest->SetLabelText( wxString::Format( "Rest: %d", pBoneController->rest ) );

	if( pBoneController->index != STUDIO_MOUTH_CONTROLLER )
	{
		m_pIndex->SetLabelText( wxString::Format( "Index: %d", pBoneController->index ) );
	}
	else
	{
		m_pIndex->SetLabelText( wxString::Format( "Index: %d (Mouth)", pBoneController->index ) );
	}

	wxArrayString types;

	for( int iType = STUDIO_CONTROL_FIRST; iType <= STUDIO_CONTROL_LAST; iType <<= 1 )
	{
		if( iType & pBoneController->type )
		{
			types.push_back( studiomodel::ControlToString( iType ) );
		}
	}

	m_pTypes->Clear();
	m_pTypes->Append( types );

	m_pBoneControllers->Select( iIndex );

	m_pBoneControllerInfo->Show( true );
}

void CBonesPanel::OnBoneChanged( wxCommandEvent& event )
{
	SetBone( m_pBones->GetSelection() );
}

void CBonesPanel::OnShowBoneController( wxCommandEvent& event )
{
	const int iIndex = m_pBoneBoneControllers->GetSelection();

	if( iIndex == wxNOT_FOUND )
		return;

	wxString szController = m_pBoneBoneControllers->GetString( iIndex );

	if( szController.IsEmpty() )
		return;

	//The bone index is at the end.
	//TODO: improve
	szController = szController.AfterLast( ' ' );

	long iController;

	if( !szController.ToLong( &iController ) )
		return;

	SetBoneController( iController );
}

void CBonesPanel::OnBoneControllerChanged( wxCommandEvent& event )
{
	SetBoneController( m_pBoneControllers->GetSelection() );
}

void CBonesPanel::OnShowBone( wxCommandEvent& event )
{
	if( !m_pBoneControllers->IsEnabled() )
		return;

	//TODO: improve
	wxString szBone = m_pControllerBone->GetLabelText().AfterLast( ' ' );

	long iBone;

	if( !szBone.ToLong( &iBone ) )
		return;

	SetBone( iBone );
}

void CBonesPanel::OnShowTypesCheatSheet( wxCommandEvent& event )
{
	ui::CStudioTypesCheatSheet dlg( this );

	dlg.ShowModal();
}
}