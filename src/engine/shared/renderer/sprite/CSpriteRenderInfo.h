#ifndef ENGINE_RENDERER_SPRITE_CSPRITERENDERINFO_H
#define ENGINE_RENDERER_SPRITE_CSPRITERENDERINFO_H

#include "utility/mathlib.h"

namespace sprite
{
struct msprite_t;

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
}

#endif //ENGINE_RENDERER_SPRITE_CSPRITERENDERINFO_H