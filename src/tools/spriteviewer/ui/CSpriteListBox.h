#ifndef TOOLS_SPRITEVIEWER_UI_CSPRITELISTBOX_H
#define TOOLS_SPRITEVIEWER_UI_CSPRITELISTBOX_H

#include <memory>
#include <vector>

#include "wxSpriteViewer.h"

#include <wx/vlbox.h>

namespace sprite
{
struct msprite_t;
struct mspriteframedesc_t;
}

namespace sprview
{
class CSpriteListBox : public wxVListBox
{
private:
	struct FrameData_t
	{
		sprite::mspriteframedesc_t* pFrameDesc;

		std::unique_ptr<wxBitmap> bitmap;

		size_t uiFrame;

		bool bIsGroup;

		FrameData_t( sprite::mspriteframedesc_t* pFrameDesc, std::unique_ptr<wxBitmap>&& bitmap, const size_t uiFrame, const bool bIsGroup = false )
			: pFrameDesc( pFrameDesc )
			, bitmap( std::move( bitmap ) )
			, uiFrame( uiFrame )
			, bIsGroup( bIsGroup )
		{
		}
	};

public:
	CSpriteListBox( wxWindow* pParent, sprite::msprite_t* pSprite = nullptr );

	void SetSprite( sprite::msprite_t* pSprite );

	double GetScale() const { return m_flScale; }

	void SetScale( double flScale )
	{
		m_flScale = flScale != 0 ? flScale : 1.0;
	}

protected:
	void OnDrawItem( wxDC& dc, const wxRect& rect, size_t n ) const override;

	wxCoord OnMeasureItem( size_t n ) const override;

	float GetTextScale() const { return m_flScale / 4.0f; }

	float GetGroupTextScale() const { return GetTextScale() * 2; }

	float GetBitmapScale() const { return m_flScale; }

private:
	sprite::msprite_t* m_pSprite;

	std::vector<std::unique_ptr<FrameData_t>> m_Frames;

	double m_flScale = 1.0;

	wxFont m_Font;
};
}

#endif //TOOLS_SPRITEVIEWER_UI_CSPRITELISTBOX_H