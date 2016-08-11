#include "../CSpriteViewerApp.h"

#include "game/entity/CSpriteEntity.h"

#include "engine/shared/sprite/sprite.h"

#include "CSpriteInfoPanel.h"

namespace sprview
{
CSpriteInfoPanel::CSpriteInfoPanel( wxWindow* pParent, CSpriteViewerApp* pHLSV )
	: CBaseControlPanel( pParent, "Sprite Info", pHLSV )
{
	m_pInfoPanel = new wxPanel( this, wxID_ANY, wxDefaultPosition, wxSize( 200, wxDefaultSize.GetHeight() ) );

	m_pType = new wxStaticText( m_pInfoPanel, wxID_ANY, "Undefined" );
	m_pTextureFormat = new wxStaticText( m_pInfoPanel, wxID_ANY, "Undefined" );
	m_pMaxWidth = new wxStaticText( m_pInfoPanel, wxID_ANY, "Undefined" );
	m_pMaxHeight = new wxStaticText( m_pInfoPanel, wxID_ANY, "Undefined" );
	m_pNumFrames = new wxStaticText( m_pInfoPanel, wxID_ANY, "Undefined" );

	//Layout
	auto pInfoSizer = new wxBoxSizer( wxVERTICAL );

	pInfoSizer->Add( m_pType, wxSizerFlags().Expand() );
	pInfoSizer->Add( m_pTextureFormat, wxSizerFlags().Expand() );
	pInfoSizer->Add( m_pMaxWidth, wxSizerFlags().Expand() );
	pInfoSizer->Add( m_pMaxHeight, wxSizerFlags().Expand() );
	pInfoSizer->Add( m_pNumFrames, wxSizerFlags().Expand() );

	m_pInfoPanel->SetSizer( pInfoSizer );

	auto pSizer = new wxBoxSizer( wxVERTICAL );

	pSizer->Add( m_pInfoPanel, wxSizerFlags().Expand().ReserveSpaceEvenIfHidden() );

	GetMainSizer()->Add( pSizer );
}

void CSpriteInfoPanel::InitializeUI()
{
	auto pEntity = m_pHLSV->GetState()->GetEntity();

	bool bSuccess = false;

	if( pEntity )
	{
		auto pSprite = pEntity->GetSprite();

		if( pSprite )
		{
			bSuccess = true;

			m_pType->SetLabelText( wxString::Format( "Type: %s", sprite::TypeToString( pSprite->type ) ) );

			wxString szValue = sprite::TexFormatToString( pSprite->texFormat );

			m_pTextureFormat->SetLabelText( wxString::Format( "Texture Format: %s", szValue.Capitalize() ) );

			m_pMaxWidth->SetLabelText( wxString::Format( "Max Width: %d", pSprite->maxwidth ) );
			m_pMaxHeight->SetLabelText( wxString::Format( "Max Height: %d", pSprite->maxheight ) );
			m_pNumFrames->SetLabelText( wxString::Format( "Number of Frames: %d", pSprite->numframes ) );
		}
	}

	m_pInfoPanel->Show( bSuccess );
}
}