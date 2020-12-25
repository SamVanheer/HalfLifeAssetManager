#pragma once

#include "engine/shared/renderer/DrawConstants.hpp"

/**
*	@defgroup SpriteRenderer Sprite Renderer
*
*	@{
*/

namespace sprite
{
struct msprite_t;
struct CSpriteRenderInfo;
struct C2DSpriteRenderInfo;

/**
*	Sprite renderer interface.
*/
class ISpriteRenderer
{
public:
	virtual ~ISpriteRenderer() = 0;

	/**
	*	Draws a sprite.
	*	@param pRenderInfo Render info.
	*	@param flags Draw flags.
	*/
	virtual void DrawSprite( const CSpriteRenderInfo* pRenderInfo, const renderer::DrawFlags_t flags ) = 0;

	/**
	*	Draws a 2D sprite. The sprite will be rescaled to the given size.
	*	@param flX X position on-screen.
	*	@param flY Y position on-screen.
	*	@param flWidth Width in pixels.
	*	@param flHeight Height in pixels.
	*	@param pSprite Sprite to draw.
	*	@param flags Draw flags.
	*/
	virtual void DrawSprite2D( const float flX, const float flY, const float flWidth, const float flHeight, const msprite_t* pSprite, const renderer::DrawFlags_t flags = renderer::DrawFlag::NONE ) = 0;

	/**
	*	Draws a 2D sprite.
	*	@param flX X position on-screen.
	*	@param flY Y position on-screen.
	*	@param pSprite Sprite to draw.
	*	@param flScale Sprite scale.
	*	@param flags Draw flags.
	*/
	virtual void DrawSprite2D( const float flX, const float flY, const msprite_t* pSprite, const float flScale = 1, const renderer::DrawFlags_t flags = renderer::DrawFlag::NONE ) = 0;

	/**
	*	Draws a 2D sprite.
	*	@param pRenderInfo Render info.
	*	@param flags Draw flags.
	*/
	virtual void DrawSprite2D( const C2DSpriteRenderInfo* pRenderInfo, const renderer::DrawFlags_t flags = renderer::DrawFlag::NONE ) = 0;
};

inline ISpriteRenderer::~ISpriteRenderer()
{
}
}

/** @} */
