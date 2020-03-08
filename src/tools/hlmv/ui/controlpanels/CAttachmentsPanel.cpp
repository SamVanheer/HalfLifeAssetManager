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

	wxStaticText* pXOrigin = new wxStaticText(m_pAttachmentInfo, wxID_ANY, "Origin X");
	wxStaticText* pYOrigin = new wxStaticText(m_pAttachmentInfo, wxID_ANY, "Origin Y");
	wxStaticText* pZOrigin = new wxStaticText(m_pAttachmentInfo, wxID_ANY, "Origin Z");

	for (int i = 0; i < 3; ++i)
	{
		m_pOrigin[i] = new wxSpinCtrlDouble(m_pAttachmentInfo, wxID_ANY);
		m_pOrigin[i]->SetRange(-DBL_MAX, DBL_MAX);
		m_pOrigin[i]->SetDigits(6);
		m_pOrigin[i]->Bind(wxEVT_SPINCTRLDOUBLE, &CAttachmentsPanel::OnOriginChanged, this);
	}

	//Layout
	auto pSizer = new wxGridBagSizer( 5, 5 );

	pSizer->Add( pAttachmentsText, wxGBPosition( 0, 0 ), wxDefaultSpan, wxEXPAND );

	pSizer->Add( m_pAttachments, wxGBPosition( 1, 0 ), wxGBSpan( 1, 1 ), wxEXPAND );

	auto pInfoSizer = new wxGridBagSizer(5, 5);

	pInfoSizer->Add(m_pName, wxGBPosition(0, 0), wxDefaultSpan, wxEXPAND);
	pInfoSizer->Add(m_pType, wxGBPosition(1, 0), wxDefaultSpan, wxEXPAND);
	pInfoSizer->Add(m_pBone, wxGBPosition(2, 0), wxDefaultSpan, wxEXPAND);

	pInfoSizer->Add(pXOrigin, wxGBPosition(0, 1), wxDefaultSpan, wxEXPAND);
	pInfoSizer->Add(pYOrigin, wxGBPosition(1, 1), wxDefaultSpan, wxEXPAND);
	pInfoSizer->Add(pZOrigin, wxGBPosition(2, 1), wxDefaultSpan, wxEXPAND);

	for (int i = 0; i < 3; ++i)
	{
		pInfoSizer->Add(m_pOrigin[i], wxGBPosition(i, 2), wxDefaultSpan, wxEXPAND);
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

			//Only enable if there are attachments
			bSuccess = pStudioHdr->numattachments > 0;
		}
	}

	SetAttachment( 0 );

	this->Enable( bSuccess );
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

	for (int i = 0; i < 3; ++i)
	{
		m_pOrigin[i]->SetValue(attachment->org[i]);
	}

	m_pAttachments->Select( iIndex );

	m_pAttachmentInfo->Show( true );
}

void CAttachmentsPanel::OnAttachmentChanged( wxCommandEvent& event )
{
	SetAttachment( m_pAttachments->GetSelection() );
}

void CAttachmentsPanel::OnOriginChanged(wxSpinDoubleEvent& event)
{
	auto pStudioHdr = m_pHLMV->GetState()->GetEntity()->GetModel()->GetStudioHeader();

	auto attachment = pStudioHdr->GetAttachment(m_pAttachments->GetSelection());

	for (int i = 0; i < 3; ++i)
	{
		attachment->org[i] = m_pOrigin[i]->GetValue();
	}

	m_pHLMV->GetState()->modelChanged = true;
}
}