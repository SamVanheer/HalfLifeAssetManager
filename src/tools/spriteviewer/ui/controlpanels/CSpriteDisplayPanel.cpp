#include <type_traits>

#include <wx/gbsizer.h>

#include "../CSpriteViewer.h"

#include "game/entity/CSpriteEntity.h"

#include "engine/shared/sprite/sprite.h"

#include "CSpriteDisplayPanel.h"

namespace sprview
{
enum
{
	wxID_TEXTUREFORMAT = wxID_HIGHEST + 1
};

wxBEGIN_EVENT_TABLE( CSpriteDisplayPanel, CBaseControlPanel )
	EVT_CHOICE( wxID_TEXTUREFORMAT, CSpriteDisplayPanel::OnTexFormatChanged )
wxEND_EVENT_TABLE()

CSpriteDisplayPanel::CSpriteDisplayPanel( wxWindow* pParent, CSpriteViewer* pHLSV )
	: CBaseControlPanel( pParent, "Sprite Display", pHLSV )
{
	wxArrayString formats;

	formats.push_back( "Use Sprite Setting" );

	for( std::underlying_type<sprite::TexFormat::TexFormat>::type format = sprite::TexFormat::FIRST; format < sprite::TexFormat::COUNT; ++format )
	{
		formats.push_back( sprite::TexFormatToString( static_cast<sprite::TexFormat::TexFormat>( format ) ) );
	}

	m_pTextureFormat = new wxChoice( this, wxID_TEXTUREFORMAT, wxDefaultPosition, wxSize( 200, wxDefaultSize.GetHeight() ), formats );

	m_pTextureFormat->Select( 0 );

	//Layout
	auto pSizer = new wxGridBagSizer( 5, 5 );

	int row = 0;
	int col = 0;

	pSizer->Add( new wxStaticText( this, wxID_ANY, "Texture Format" ), wxGBPosition( row++, col ), wxGBSpan( 1, 2 ), wxEXPAND );

	pSizer->Add( m_pTextureFormat, wxGBPosition( row++, col ), wxGBSpan( 1, 2 ), wxEXPAND );

	GetMainSizer()->Add( pSizer );
}

void CSpriteDisplayPanel::InitializeUI()
{
	auto pEntity = m_pHLSV->GetState()->GetEntity();

	bool bSuccess = false;

	if( pEntity )
	{
		auto pSprite = pEntity->GetSprite();

		if( pSprite )
		{
			bSuccess = true;
		}
	}

	m_pTextureFormat->Select( 0 );
}

void CSpriteDisplayPanel::OnTexFormatChanged( wxCommandEvent& event )
{
	auto texFormat = m_pTextureFormat->GetSelection();

	const auto format = static_cast<sprite::TexFormat::TexFormat>( texFormat - 1 );

	if( texFormat == wxNOT_FOUND || texFormat == 0 || format < sprite::TexFormat::FIRST || format >= sprite::TexFormat::LAST )
	{
		m_pHLSV->GetState()->ClearTexFormatOverride();
	}
	else
	{
		m_pHLSV->GetState()->SetTexFormatOverride( format );
	}
}
}