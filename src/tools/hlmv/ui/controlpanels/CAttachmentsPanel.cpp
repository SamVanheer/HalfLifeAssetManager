#include <wx/gbsizer.h>

#include "shared/renderer/studiomodel/IStudioModelRenderer.h"

#include "../CModelViewerApp.h"
#include "../../CHLMVState.h"

#include "CAttachmentsPanel.h"

namespace hlmv
{
wxBEGIN_EVENT_TABLE( CAttachmentsPanel, CBaseControlPanel )
	EVT_CHOICE( wxID_ATTACH_ATTACHMENT, CAttachmentsPanel::OnAttachmentChanged )
wxEND_EVENT_TABLE()

CAttachmentsPanel::CAttachmentsPanel( wxWindow* pParent, CModelViewerApp* const pHLMV )
	: CBaseControlPanel( pParent, "Attachments", pHLMV )
{
	auto pElemParent = GetElementParent();

	auto pAttachmentsText = new wxStaticText( pElemParent, wxID_ANY, "Attachments:" );

	m_pAttachments = new wxChoice( pElemParent, wxID_ATTACH_ATTACHMENT, wxDefaultPosition, wxSize( 125, wxDefaultSize.GetHeight() ) );

	m_pAttachmentInfo = new wxPanel( pElemParent );

	m_pName = new wxStaticText( m_pAttachmentInfo, wxID_ANY, "Undefined", wxDefaultPosition, wxSize( 300, wxDefaultSize.GetHeight() ) );
	m_pType = new wxStaticText( m_pAttachmentInfo, wxID_ANY, "Undefined", wxDefaultPosition, wxSize( 300, wxDefaultSize.GetHeight() ) );
	m_pBone = new wxStaticText( m_pAttachmentInfo, wxID_ANY, "Undefined", wxDefaultPosition, wxSize( 300, wxDefaultSize.GetHeight() ) );
	m_pOrigin = new wxStaticText( m_pAttachmentInfo, wxID_ANY, "Undefined", wxDefaultPosition, wxSize( 300, wxDefaultSize.GetHeight() ) );

	for( int iIndex = 0; iIndex < STUDIO_ATTACH_NUM_VECTORS; ++iIndex )
	{
		m_pVectors[ iIndex ] = new wxStaticText( m_pAttachmentInfo, wxID_ANY, "Undefined", wxDefaultPosition, wxSize( 300, wxDefaultSize.GetHeight() ) );
	}

	//Layout
	auto pSizer = new wxGridBagSizer( 5, 5 );

	pSizer->Add( pAttachmentsText, wxGBPosition( 0, 0 ), wxDefaultSpan, wxEXPAND );

	pSizer->Add( m_pAttachments, wxGBPosition( 1, 0 ), wxGBSpan( 1, 1 ), wxEXPAND );

	auto pInfoSizer = new wxBoxSizer( wxVERTICAL );

	pInfoSizer->Add( m_pName );
	pInfoSizer->Add( m_pType );
	pInfoSizer->Add( m_pBone );
	pInfoSizer->Add( m_pOrigin );

	for( int iIndex = 0; iIndex < STUDIO_ATTACH_NUM_VECTORS; ++iIndex )
	{
		pInfoSizer->Add( m_pVectors[ iIndex ] );
	}

	m_pAttachmentInfo->SetSizer( pInfoSizer );

	pSizer->Add( m_pAttachmentInfo, wxGBPosition( 0, 1 ), wxGBSpan( 3, 1 ), wxEXPAND | wxRESERVE_SPACE_EVEN_IF_HIDDEN );

	GetMainSizer()->Add( pSizer );
}

CAttachmentsPanel::~CAttachmentsPanel()
{
}

void CAttachmentsPanel::InitializeUI()
{
	auto pEntity = m_pHLMV->GetState()->GetEntity();

	bool bSuccess = false;

	m_pAttachments->Clear();

	if( pEntity )
	{
		if( auto pModel = pEntity->GetModel() )
		{
			wxArrayString attachments;

			const auto pStudioHdr = pModel->GetStudioHeader();

			for( int iAttachment = 0; iAttachment < pStudioHdr->numattachments; ++iAttachment )
			{
				//Can't use the attachment name because it's unused.
				attachments.push_back( wxString::Format( "Attachment: %d", iAttachment + 1 ) );
			}

			m_pAttachments->Append( attachments );

			bSuccess = true;
		}
	}

	SetAttachment( 0 );

	m_pAttachments->Enable( bSuccess );
}

void CAttachmentsPanel::OnPostDraw( studiomdl::IStudioModelRenderer& renderer, const studiomdl::CModelRenderInfo& info )
{
	renderer.DrawSingleAttachment( m_pAttachments->GetSelection() );
}

void CAttachmentsPanel::SetAttachment( int iIndex )
{
	assert( iIndex >= 0 );

	auto pEntity = m_pHLMV->GetState()->GetEntity();

	if( !pEntity )
	{
		m_pAttachmentInfo->Show( false );
		return;
	}

	auto pModel = pEntity->GetModel();

	const auto pStudioHdr = pModel->GetStudioHeader();

	if( pStudioHdr->numattachments <= 0 )
	{
		m_pAttachmentInfo->Show( false );
		return;
	}

	if( iIndex < 0 || iIndex >= pStudioHdr->numattachments )
		iIndex = 0;

	const auto attachment = pStudioHdr->GetAttachment( iIndex );

	//Signal empty names by setting it to (empty) and making it red.
	m_pName->SetLabelText( wxString::Format( "Name: %s", *attachment->name ? attachment->name : "(empty)" ) );
	m_pName->SetForegroundColour( *attachment->name ? wxColor( 0, 0, 0 ) : wxColor( 255, 0, 0 ) );

	m_pType->SetLabelText( wxString::Format( "Type: %d", attachment->type ) );
	m_pBone->SetLabelText( wxString::Format( "Bone: %d", attachment->bone ) );
	m_pOrigin->SetLabelText( wxString::Format( "Origin: %f %f %f", attachment->org.x, attachment->org.y, attachment->org.z ) );

	for( int iVec = 0; iVec < STUDIO_ATTACH_NUM_VECTORS; ++iVec )
	{
		m_pVectors[ iVec ]->SetLabelText( wxString::Format( "Vector %d: %f %f %f", 
															iVec, attachment->vectors[ iVec ].x, 
															attachment->vectors[ iVec ].y, 
															attachment->vectors[ iVec ].z ) );
	}

	m_pAttachments->Select( iIndex );

	m_pAttachmentInfo->Show( true );
}

void CAttachmentsPanel::OnAttachmentChanged( wxCommandEvent& event )
{
	SetAttachment( m_pAttachments->GetSelection() );
}
}