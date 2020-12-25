#pragma once

#include "utility/mathlib.hpp"

#include "engine/shared/sprite/sprite.hpp"

namespace sprite
{
/**
*	Data structure used to pass sprite render info into the engine.
*	TODO: this should only explicitly declare variables for sprite specific settings. Common settings should be accessed through a shared interface.
*/
struct CSpriteRenderInfo final
{
	glm::vec3 vecOrigin;
	glm::vec3 vecAngles;
	glm::vec3 vecScale;

	msprite_t* pSprite;

	float flTransparency;

	float flFrame;

	sprite::Type::Type type = sprite::Type::VP_PARALLEL_UPRIGHT;

	bool bOverrideType = false;
};

/**
*	Data structure used to pass 2D sprite render info into the engine.
*/
struct C2DSpriteRenderInfo final
{
	glm::vec2 vecPos;
	glm::vec2 vecScale;

	msprite_t* pSprite;

	float flTransparency;

	float flFrame;

	sprite::TexFormat::TexFormat texFormat = sprite::TexFormat::SPR_NORMAL;

	bool bOverrideTexFormat = false;
};
}
