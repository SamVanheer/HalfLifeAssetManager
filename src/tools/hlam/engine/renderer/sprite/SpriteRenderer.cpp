#include <cassert>
#include <cmath>

#include <glm/vec4.hpp>

#include "core/shared/Logging.hpp"
#include "core/shared/WorldTime.hpp"

#include "graphics/OpenGL.hpp"

#include "engine/shared/renderer/sprite/SpriteRenderInfo.hpp"

#include "engine/shared/sprite/SpriteFileFormat.hpp"

#include "engine/renderer/sprite/SpriteRenderer.hpp"

namespace sprite
{
SpriteRenderer::SpriteRenderer(WorldTime* worldTime)
	: _worldTime(worldTime)
{
}

SpriteRenderer::~SpriteRenderer() = default;

void SpriteRenderer::DrawSprite(const SpriteRenderInfo& renderInfo, const renderer::DrawFlags flags)
{
	const auto sprite = renderInfo.Sprite;

	assert(sprite);

	if (!sprite)
	{
		Error("CSpriteRenderer::DrawSprite2D: Null sprite!\n");
		return;
	}

	const auto& framedesc = sprite->frames[static_cast<int>(renderInfo.Frame)];
	//TODO: get correct frame when it's a group.
	const auto& frame = framedesc.frameptr;

	const sprite::Type::Type* typeOverride = renderInfo.OverrideType ? &renderInfo.Type : nullptr;

	DrawSprite(renderInfo.Origin, {frame->width, frame->height}, sprite, renderInfo.Frame, flags, typeOverride);
}

void SpriteRenderer::DrawSprite2D(const float x, const float y, const float width, const float height,
	const msprite_t* sprite, const renderer::DrawFlags flags)
{
	const float frameIndex = static_cast<float>(fmod(_worldTime->GetTime() * DEFAULT_FRAMERATE, sprite->numframes));

	//TODO: calculate frame
	DrawSprite({x, y, 0}, {width, height}, sprite, frameIndex, flags);
}

void SpriteRenderer::DrawSprite2D(const float x, const float y,
	const msprite_t* sprite, const float scale, const renderer::DrawFlags flags)
{
	assert(sprite);

	const int frameIndex = static_cast<int>(fmod(_worldTime->GetTime() * DEFAULT_FRAMERATE, sprite->numframes));

	//TODO: calculate frame
	const auto& framedesc = sprite->frames[frameIndex];

	mspriteframe_t* frame;

	if (framedesc.type == spriteframetype_t::SINGLE)
	{
		frame = framedesc.frameptr;
	}
	else
	{
		auto pGroup = framedesc.GetGroup();

		//TODO: get correct frame.
		frame = pGroup->frames[0];
	}

	DrawSprite2D(x, y, static_cast<float>(frame->width * scale), static_cast<float>(frame->height * scale), sprite, flags);
}

void SpriteRenderer::DrawSprite2D(const Sprite2DRenderInfo& renderInfo, const renderer::DrawFlags flags)
{
	const auto sprite = renderInfo.Sprite;

	assert(sprite);

	if (!sprite)
	{
		Error("CSpriteRenderer::DrawSprite2D: Null sprite!\n");
		return;
	}

	const auto& framedesc = sprite->frames[static_cast<int>(renderInfo.Frame)];

	mspriteframe_t* frame;

	if (framedesc.type == spriteframetype_t::SINGLE)
	{
		frame = framedesc.frameptr;
	}
	else
	{
		auto pGroup = framedesc.GetGroup();

		//TODO: get correct frame.
		frame = pGroup->frames[0];
	}

	const sprite::TexFormat::TexFormat* texFormatOverride = renderInfo.OverrideTexFormat ? &renderInfo.TexFormat : nullptr;

	DrawSprite(glm::vec3(renderInfo.Pos, 0),
		glm::vec2(renderInfo.Scale.x * frame->width, renderInfo.Scale.y * frame->height),
		renderInfo.Sprite, renderInfo.Frame, flags, nullptr, texFormatOverride);
}

void SpriteRenderer::DrawSprite(const glm::vec3& origin, const glm::vec2& size,
	const msprite_t* sprite, const float frameIndex,
	const renderer::DrawFlags flags, const sprite::Type::Type* typeOverride, const sprite::TexFormat::TexFormat* texFormatOverride)
{
	assert(sprite);

	const auto& framedesc = sprite->frames[static_cast<int>(floor(frameIndex))];

	mspriteframe_t* frame;

	if (framedesc.type == spriteframetype_t::SINGLE)
	{
		frame = framedesc.frameptr;
	}
	else
	{
		auto pGroup = framedesc.GetGroup();

		float* pflIntervals = pGroup->intervals;

		double flInt;

		const float flFraction = static_cast<float>(modf(frameIndex, &flInt));

		int iIndex;

		for (iIndex = 0; iIndex < (pGroup->numframes - 1); ++iIndex)
		{
			if (pflIntervals[iIndex] > flFraction)
				break;
		}

		assert(iIndex >= 0);

		frame = pGroup->frames[iIndex];
	}

	glEnable(GL_TEXTURE_2D);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glBindTexture(GL_TEXTURE_2D, frame->gl_texturenum);

	//TODO: set up the sprite's orientation in the world according to its type.
	//TODO: the size of the sprite should change based on its distance from the viewer.

	const sprite::TexFormat::TexFormat texFormat = texFormatOverride ? *texFormatOverride : sprite->texFormat;

	switch (texFormat)
	{
	default:
	case TexFormat::SPR_NORMAL:
	{
		glTexEnvi(GL_TEXTURE_2D, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glDisable(GL_BLEND);
		break;
	}

	case TexFormat::SPR_ADDITIVE:
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		break;
	}

	case TexFormat::SPR_INDEXALPHA:
	case TexFormat::SPR_ALPHTEST:
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		break;
	}
	}

	if (texFormat == TexFormat::SPR_ALPHTEST)
	{
		glEnable(GL_ALPHA_TEST);
		glAlphaFunc(GL_GREATER, 0.0f);
	}
	else
	{
		glDisable(GL_ALPHA_TEST);
	}

	const glm::vec4 vecRect{origin.x - size.x / 2, origin.y - size.y / 2, origin.x + size.x / 2, origin.y + size.y / 2};

	if (!(flags & renderer::DrawFlag::NODRAW))
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);
		glShadeModel(GL_SMOOTH);
		glColor4f(1, 1, 1, 1);

		glBegin(GL_TRIANGLE_STRIP);

		glTexCoord2f(0, 0);
		glVertex3f(vecRect.x, vecRect.y, origin.z);

		glTexCoord2f(1, 0);
		glVertex3f(vecRect.z, vecRect.y, origin.z);

		glTexCoord2f(0, 1);
		glVertex3f(vecRect.x, vecRect.w, origin.z);

		glTexCoord2f(1, 1);
		glVertex3f(vecRect.z, vecRect.w, origin.z);

		glEnd();
	}

	if (flags & renderer::DrawFlag::WIREFRAME_OVERLAY)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
		glColor4f(1, 1, 1, 1);

		glBegin(GL_TRIANGLE_STRIP);

		glVertex3f(vecRect.x, vecRect.y, origin.z);

		glVertex3f(vecRect.z, vecRect.y, origin.z);

		glVertex3f(vecRect.x, vecRect.w, origin.z);

		glVertex3f(vecRect.z, vecRect.w, origin.z);

		glEnd();
	}
}
}