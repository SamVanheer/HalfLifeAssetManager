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
	wxID_TYPE = wxID_HIGHEST + 1,
	wxID_TEXTUREFORMAT
};

wxBEGIN_EVENT_TABLE( CSpriteDisplayPanel, CBaseControlPanel )
	EVT_CHOICE( wxID_TYPE, CSpriteDisplayPanel::OnTypeChanged )
	EVT_CHOICE( wxID_TEXTUREFORMAT, CSpriteDisplayPanel::OnTexFormatChanged )
wxEND_EVENT_TABLE()

namespace
{
static const wxString USE_SPRITE_SETTING_STRING( "Use Sprite Setting" );
}

CSpriteDisplayPanel::CSpriteDisplayPanel( wxWindow* pParent, CSpriteViewer* pHLSV )
	: CBaseControlPanel( pParent, "Sprite Display", pHLSV )
{
	{
		wxArrayString types;

		types.push_back( USE_SPRITE_SETTING_STRING );

		for( std::underlying_type<sprite::Type::Type>::type type = sprite::TexFormat::FIRST; type < sprite::Type::COUNT; ++type )
		{
			types.push_back( sprite::TypeToString( static_cast<sprite::Type::Type>( type ) ) );
		}

		m_pType = new wxChoice( this, wxID_TYPE, wxDefaultPosition, wxSize( 200, wxDefaultSize.GetHeight() ), types );

		m_pType->Select( 0 );
	}

	{
		wxArrayString formats;

		formats.push_back( USE_SPRITE_SETTING_STRING );

		for( std::underlying_type<sprite::TexFormat::TexFormat>::type format = sprite::TexFormat::FIRST; format < sprite::TexFormat::COUNT; ++format )
		{
			formats.push_back( sprite::TexFormatToString( static_cast<sprite::TexFormat::TexFormat>( format ) ) );
		}

		m_pTextureFormat = new wxChoice( this, wxID_TEXTUREFORMAT, wxDefaultPosition, wxSize( 200, wxDefaultSize.GetHeight() ), formats );

		m_pTextureFormat->Select( 0 );
	}

	//Layout
	auto pSizer = new wxGridBagSizer( 5, 5 );

	int row = 0;
	int col = 0;

	pSizer->Add( new wxStaticText( this, wxID_ANY, "Orientation Type:" ), wxGBPosition( row++, col ), wxGBSpan( 1, 2 ), wxEXPAND );

	pSizer->Add( m_pType, wxGBPosition( row++, col ), wxGBSpan( 1, 2 ), wxEXPAND );

	pSizer->Add( new wxStaticText( this, wxID_ANY, "Texture Format:" ), wxGBPosition( row++, col ), wxGBSpan( 1, 2 ), wxEXPAND );

	pSizer->Add( m_pTextureFormat, wxGBPosition( row++, col ), wxGBSpan( 1, 2 ), wxEXPAND );

	GetMainSizer()->Add( pSizer, wxSizerFlags().Expand() );
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

	m_pType->Select( 0 );
	m_pTextureFormat->Select( 0 );
}

void CSpriteDisplayPanel::OnTypeChanged( wxCommandEvent& event )
{
	auto type = m_pTextureFormat->GetSelection();

	const auto orientationType = static_cast<sprite::Type::Type>( type - 1 );

	if( type == wxNOT_FOUND || type == 0 || orientationType < sprite::TexFormat::FIRST || orientationType >= sprite::TexFormat::LAST )
	{
		m_pHLSV->GetState()->ClearTypeOverride();
	}
	else
	{
		m_pHLSV->GetState()->SetTypeOverride( orientationType );
	}
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