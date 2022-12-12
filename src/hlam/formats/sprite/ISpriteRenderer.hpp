#pragma once

#include "formats/DrawConstants.hpp"

class QOpenGLFunctions_1_1;

/**
*	@defgroup SpriteRenderer Sprite Renderer
*
*	@{
*/

namespace sprite
{
struct msprite_t;
struct SpriteRenderInfo;
struct Sprite2DRenderInfo;

/**
*	Sprite renderer interface.
*/
class ISpriteRenderer
{
public:
	virtual ~ISpriteRenderer() {}

	virtual void SetOpenGLFunctions(QOpenGLFunctions_1_1* openglFunctions) = 0;

	virtual void DrawSprite(const SpriteRenderInfo& renderInfo, const renderer::DrawFlags flags) = 0;

	/**
	*	Draws a 2D sprite. The sprite will be rescaled to the given size.
	*	@param x X position on-screen.
	*	@param y Y position on-screen.
	*	@param width Width in pixels.
	*	@param height Height in pixels.
	*	@param sprite Sprite to draw.
	*	@param flags Draw flags.
	*/
	virtual void DrawSprite2D(const float x, const float y, const float width, const float height,
		const msprite_t* sprite, const renderer::DrawFlags flags = renderer::DrawFlag::NONE) = 0;

	/**
	*	Draws a 2D sprite.
	*	@param x X position on-screen.
	*	@param y Y position on-screen.
	*	@param sprite Sprite to draw.
	*	@param scale Sprite scale.
	*	@param flags Draw flags.
	*/
	virtual void DrawSprite2D(const float x, const float y,
		const msprite_t* sprite, const float scale = 1, const renderer::DrawFlags flags = renderer::DrawFlag::NONE) = 0;

	virtual void DrawSprite2D(const Sprite2DRenderInfo& renderInfo, const renderer::DrawFlags flags = renderer::DrawFlag::NONE) = 0;
};
}

/** @} */
