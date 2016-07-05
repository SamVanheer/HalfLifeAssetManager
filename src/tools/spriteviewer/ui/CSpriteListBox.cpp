#include "core/shared/Const.h"

#include "graphics/OpenGL.h"

#include "engine/shared/sprite/sprite.h"

#include "CSpriteListBox.h"

namespace sprview
{
namespace
{
static const wxString GROUP_TITLE_TEXT( "Group frame #%u" );
}

CSpriteListBox::CSpriteListBox( wxWindow* pParent, sprite::msprite_t* pSprite )
	: wxVListBox( pParent )
	, m_pSprite( nullptr )
	, m_Font( 16, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, wxT( "Arial" ) )
{
	SetSprite( pSprite );
}

static std::unique_ptr<wxBitmap> LoadSpriteAsBitmap( std::vector<byte> rgbBuffer, std::vector<byte> alphaBuffer, sprite::mspriteframe_t* pFrame )
{
	const int iNumPixels = pFrame->width * pFrame->height;

	const GLsizei iSize = iNumPixels * 4;
	const GLsizei iAlphaSize = iNumPixels;

	if( iSize > 0 )
	{
		//Get the image contents from the GPU.
		rgbBuffer.resize( iSize );
		alphaBuffer.resize( iAlphaSize );

		glBindTexture( GL_TEXTURE_2D, pFrame->gl_texturenum );
		glGetnTexImage( GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgbBuffer.size(), rgbBuffer.data() );

		byte* pInData = rgbBuffer.data() + 3;
		byte* pOutData = alphaBuffer.data();

		//No way to get the alpha channel on its own, so pull it out.
		for( int x = 0; x < pFrame->width; ++x )
		{
			for( int y = 0; y < pFrame->height; ++y, pInData += 4 )
			{
				*pOutData++ = *pInData;
			}
		}

		//Now remove the alpha channel.
		for( int iCount = 0; iCount < iNumPixels; ++iCount )
		{
			rgbBuffer.erase( rgbBuffer.end() - ( ( iCount * 3 ) + 1 ) );
		}

		//TODO: figure out how to toggle the alpha channel - Solokiller
		wxImage image( pFrame->width, pFrame->height, rgbBuffer.data()/*, alphaBuffer.data()*/, true );

		if( image.IsOk() )
		{
			return std::make_unique<wxBitmap>( image );
		}
		else
		{
			//TODO: error handling.
		}
	}
	else
	{
		//TODO: error handling.
	}

	return nullptr;
}

void CSpriteListBox::SetSprite( sprite::msprite_t* pSprite )
{
	if( m_pSprite )
	{
		m_Frames.clear();
		m_Frames.shrink_to_fit();
	}

	m_pSprite = pSprite;

	if( m_pSprite )
	{
		m_Frames.reserve( m_pSprite->numframes );

		std::vector<byte> rgbBuffer;
		std::vector<byte> alphaBuffer;

		size_t uiNumFrames = 0;

		for( int iIndex = 0; iIndex < m_pSprite->numframes; ++iIndex )
		{
			auto pFrameDesc = &m_pSprite->frames[ iIndex ];

			if( pFrameDesc->type == sprite::spriteframetype_t::SINGLE )
			{
				auto pFrame = pFrameDesc->frameptr;

				auto bitmap = LoadSpriteAsBitmap( rgbBuffer, alphaBuffer, pFrame );

				if( bitmap )
				{
					m_Frames.push_back( std::make_unique<FrameData_t>( pFrameDesc, std::move( bitmap ), iIndex ) );

					++uiNumFrames;
				}
				else
				{
					//TODO: error handling.
				}
			}
			else
			{
				auto pFrameGroup = reinterpret_cast<sprite::mspritegroup_t*>( pFrameDesc->frameptr );

				for( int iGroupIndex = 0; iGroupIndex < pFrameGroup->numframes; ++iGroupIndex )
				{
					auto bitmap = LoadSpriteAsBitmap( rgbBuffer, alphaBuffer, pFrameGroup->frames[ iGroupIndex ] );

					if( bitmap )
					{
						m_Frames.emplace_back( std::make_unique<FrameData_t>( pFrameDesc, std::move( bitmap ), iGroupIndex, true ) );

						++uiNumFrames;
					}
					else
					{
						//TODO: error handling.
					}
				}
			}
		}

		//Must be set after getting all of the bitmaps loaded, since it calls OnMeasureItem.
		SetItemCount( uiNumFrames );

		Refresh();

		//TODO: Store this off somewhere. IIRC The engine uses this value to perform simple visibility testing.
		const float flBounding = sqrt( ( m_pSprite->maxwidth / 2.0 ) * ( m_pSprite->maxwidth / 2.0 ) + ( m_pSprite->maxheight / 2.0 ) * ( m_pSprite->maxheight / 2.0 ) );
	}
	else
	{
		SetItemCount( 0 );
	}
}

static void DrawText( wxDC& dc, const wxString& szText, wxPoint& textCoord )
{
	dc.DrawText( szText, textCoord );

	textCoord.y += dc.GetTextExtent( szText ).GetHeight();
}

void CSpriteListBox::OnDrawItem( wxDC& dc, const wxRect& rect, size_t n ) const
{
	dc.SetFont( m_Font );

	auto& frame = m_Frames[ n ];

	auto& bitmap = frame->bitmap;

	double x, y;

	dc.GetLogicalScale( &x, &y );

	dc.SetLogicalScale( GetBitmapScale(), GetBitmapScale() );

	//Have to remap the unscaled coordinates to our scaled ones.
	wxPoint topLeft( dc.DeviceToLogicalXRel( rect.GetLeft() ), dc.DeviceToLogicalYRel( rect.GetTop() ) );

	wxCoord xOffset = 0;
	wxCoord yOffset = 0;

	sprite::mspriteframe_t* pFrame;

	if( frame->bIsGroup )
	{
		pFrame = frame->pFrameDesc->GetGroup()->frames[ frame->uiFrame ];

		if( frame->uiFrame == 0 )
		{
			dc.SetLogicalScale( GetGroupTextScale(), GetGroupTextScale() );

			wxPoint topLeftText( dc.DeviceToLogicalXRel( rect.GetLeft() ), dc.DeviceToLogicalYRel( rect.GetTop() ) );

			const wxString szText = wxString::Format( GROUP_TITLE_TEXT, n );
			dc.DrawText( szText, topLeftText );

			int height = dc.GetTextExtent( szText ).GetHeight();

			yOffset = dc.LogicalToDeviceYRel( height );

			dc.SetLogicalScale( GetBitmapScale(), GetBitmapScale() );

			topLeft.y += dc.DeviceToLogicalYRel( yOffset );
		}

		//Offset the group frames.
		xOffset = 20;
		topLeft.x += xOffset;
	}
	else
	{
		pFrame = frame->pFrameDesc->GetFrame();
	}

	dc.DrawBitmap( *bitmap, topLeft, false );

	const wxCoord scaledWidth = dc.LogicalToDeviceXRel( bitmap->GetWidth() );
	const wxCoord scaledHeight = dc.LogicalToDeviceYRel( bitmap->GetHeight() );

	wxPoint textCoord = rect.GetTopLeft();

	textCoord.x += scaledWidth + dc.LogicalToDeviceXRel( xOffset );
	textCoord.y += yOffset;

	dc.SetLogicalScale( GetTextScale(), GetTextScale() );

	//Adjust position to match the text scale.
	textCoord.x = dc.DeviceToLogicalXRel( textCoord.x );
	textCoord.y = dc.DeviceToLogicalYRel( textCoord.y );

	//Offset the text a bit.
	textCoord.x += 20;

	wxString szText = wxString::Format( "Frame index: %u", frame->uiFrame );

	dc.DrawText( szText, textCoord );

	textCoord.y += dc.GetTextExtent( szText ).GetHeight();

	DrawText( dc, wxString::Format( "Dimensions: %d x %d", pFrame->width, pFrame->height ), textCoord );

	DrawText( dc, wxString::Format( "Origin: %.1f, %.1f", pFrame->left, pFrame->up ), textCoord );

	if( frame->bIsGroup )
	{
		sprite::mspritegroup_t* pGroup = reinterpret_cast<sprite::mspritegroup_t*>( frame->pFrameDesc->frameptr );

		szText = wxString::Format( "Interval: %.2f", pGroup->intervals[ frame->uiFrame ] );

		dc.DrawText( szText, textCoord );

		textCoord.y += dc.GetTextExtent( szText ).GetHeight();
	}

	dc.SetLogicalScale( x, y );
}

wxCoord CSpriteListBox::OnMeasureItem( size_t n ) const
{
	wxCoord height = 0;

	auto& frame = m_Frames[ n ];

	if( frame->bIsGroup )
	{
		if( frame->uiFrame == 0 )
		{
			wxClientDC dc( const_cast<CSpriteListBox*>( this ) );

			sprite::mspritegroup_t* pGroup = reinterpret_cast<sprite::mspritegroup_t*>( frame->pFrameDesc->frameptr );

			dc.SetFont( m_Font );

			height += dc.GetTextExtent( GROUP_TITLE_TEXT ).GetHeight() * GetGroupTextScale();
		}
	}

	height += frame->bitmap->GetHeight() * GetBitmapScale();

	//Give it some space between each frame.
	return height + 20;
}
}