#ifndef ENGINE_RENDERER_SPRITE_CSPRITERENDERINFO_H
#define ENGINE_RENDERER_SPRITE_CSPRITERENDERINFO_H

#include "utility/mathlib.h"

#include "engine/shared/sprite/sprite.h"

namespace sprite
{
/**
*	Data structure used to pass sprite render info into the engine.
*	TODO: this should only explicitly declare variables for sprite specific settings. Common settings should be accessed through a shared interface.
*/
struct CSpriteRenderInfo final
{
	Vector vecOrigin;
	Vector vecAngles;
	Vector vecScale;

	msprite_t* pSprite;

	float flTransparency;

	float flFrame;
};

/**
*	Data structure used to pass 2D sprite render info into the engine.
*/
struct C2DSpriteRenderInfo final
{
	Vector2D vecPos;
	Vector2D vecScale;

	msprite_t* pSprite;

	float flTransparency;

	float flFrame;

	sprite::TexFormat::TexFormat texFormat = sprite::TexFormat::SPR_NORMAL;

	bool bOverrideTexFormat = false;
};
}

#endif //ENGINE_RENDERER_SPRITE_CSPRITERENDERINFO_H