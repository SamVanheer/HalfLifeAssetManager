#pragma once

#include "utility/mathlib.hpp"

#include "engine/shared/sprite/sprite.hpp"

namespace sprite
{
/**
*	Data structure used to pass sprite render info into the engine.
*	TODO: this should only explicitly declare variables for sprite specific settings. Common settings should be accessed through a shared interface.
*/
struct SpriteRenderInfo final
{
	glm::vec3 Origin;
	glm::vec3 Angles;
	glm::vec3 Scale;

	msprite_t* Sprite;

	float Transparency;

	float Frame;

	sprite::Type::Type Type = sprite::Type::VP_PARALLEL_UPRIGHT;

	bool OverrideType = false;
};

/**
*	Data structure used to pass 2D sprite render info into the engine.
*/
struct Sprite2DRenderInfo final
{
	glm::vec2 Pos;
	glm::vec2 Scale;

	msprite_t* Sprite;

	float Transparency;

	float Frame;

	sprite::TexFormat::TexFormat TexFormat = sprite::TexFormat::SPR_NORMAL;

	bool OverrideTexFormat = false;
};
}
