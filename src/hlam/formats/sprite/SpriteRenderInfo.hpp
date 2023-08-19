#pragma once

#include "formats/sprite/SpriteFileFormat.hpp"

#include "utility/mathlib.hpp"

namespace sprite
{
/**
*	Data structure used to pass sprite render info into the engine.
*	TODO: this should only explicitly declare variables for sprite specific settings. Common settings should be accessed through a shared interface.
*/
struct SpriteRenderInfo final
{
	glm::vec3 Origin{0};
	glm::vec3 Angles{0};
	glm::vec3 Scale{0};

	msprite_t* Sprite = nullptr;

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
