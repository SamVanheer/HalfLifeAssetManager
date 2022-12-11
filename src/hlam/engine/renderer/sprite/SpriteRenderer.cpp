#include <cassert>
#include <cmath>

#include <spdlog/spdlog.h>

#include <glm/vec4.hpp>

#include "graphics/OpenGL.hpp"

#include "engine/shared/renderer/sprite/SpriteRenderInfo.hpp"

#include "engine/shared/sprite/SpriteFileFormat.hpp"

#include "engine/renderer/sprite/SpriteRenderer.hpp"

#include "utility/WorldTime.hpp"

namespace sprite
{
SpriteRenderer::SpriteRenderer(const std::shared_ptr<spdlog::logger>& logger, WorldTime* worldTime)
	: _logger(logger)
	, _worldTime(worldTime)
{
}

SpriteRenderer::~SpriteRenderer() = default;

void SpriteRenderer::DrawSprite(const SpriteRenderInfo& renderInfo, const renderer::DrawFlags flags)
{
	const auto sprite = renderInfo.Sprite;

	assert(sprite);

	if (!sprite)
	{
		SPDLOG_LOGGER_CALL(_logger, spdlog::level::err, "Null sprite!");
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
		SPDLOG_LOGGER_CALL(_logger, spdlog::level::err, "Null sprite!");
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

	_openglFunctions->glEnable(GL_TEXTURE_2D);
	_openglFunctions->glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	_openglFunctions->glBindTexture(GL_TEXTURE_2D, frame->gl_texturenum);

	//TODO: set up the sprite's orientation in the world according to its type.
	//TODO: the size of the sprite should change based on its distance from the viewer.

	const sprite::TexFormat::TexFormat texFormat = texFormatOverride ? *texFormatOverride : sprite->texFormat;

	switch (texFormat)
	{
	default:
	case TexFormat::SPR_NORMAL:
	{
		_openglFunctions->glTexEnvi(GL_TEXTURE_2D, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		_openglFunctions->glDisable(GL_BLEND);
		break;
	}

	case TexFormat::SPR_ADDITIVE:
	{
		_openglFunctions->glEnable(GL_BLEND);
		_openglFunctions->glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		break;
	}

	case TexFormat::SPR_INDEXALPHA:
	case TexFormat::SPR_ALPHTEST:
	{
		_openglFunctions->glEnable(GL_BLEND);
		_openglFunctions->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		break;
	}
	}

	if (texFormat == TexFormat::SPR_ALPHTEST)
	{
		_openglFunctions->glEnable(GL_ALPHA_TEST);
		_openglFunctions->glAlphaFunc(GL_GREATER, 0.0f);
	}
	else
	{
		_openglFunctions->glDisable(GL_ALPHA_TEST);
	}

	const glm::vec4 vecRect{origin.x - size.x / 2, origin.y - size.y / 2, origin.x + size.x / 2, origin.y + size.y / 2};

	if (!(flags & renderer::DrawFlag::NODRAW))
	{
		_openglFunctions->glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		_openglFunctions->glEnable(GL_TEXTURE_2D);
		_openglFunctions->glEnable(GL_CULL_FACE);
		_openglFunctions->glEnable(GL_DEPTH_TEST);
		_openglFunctions->glShadeModel(GL_SMOOTH);
		_openglFunctions->glColor4f(1, 1, 1, 1);

		_openglFunctions->glBegin(GL_TRIANGLE_STRIP);

		_openglFunctions->glTexCoord2f(0, 0);
		_openglFunctions->glVertex3f(vecRect.x, vecRect.y, origin.z);

		_openglFunctions->glTexCoord2f(1, 0);
		_openglFunctions->glVertex3f(vecRect.z, vecRect.y, origin.z);

		_openglFunctions->glTexCoord2f(0, 1);
		_openglFunctions->glVertex3f(vecRect.x, vecRect.w, origin.z);

		_openglFunctions->glTexCoord2f(1, 1);
		_openglFunctions->glVertex3f(vecRect.z, vecRect.w, origin.z);

		_openglFunctions->glEnd();
	}

	if (flags & renderer::DrawFlag::WIREFRAME_OVERLAY)
	{
		_openglFunctions->glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		_openglFunctions->glDisable(GL_TEXTURE_2D);
		_openglFunctions->glDisable(GL_CULL_FACE);
		_openglFunctions->glDisable(GL_DEPTH_TEST);
		_openglFunctions->glColor4f(1, 1, 1, 1);

		_openglFunctions->glBegin(GL_TRIANGLE_STRIP);

		_openglFunctions->glVertex3f(vecRect.x, vecRect.y, origin.z);

		_openglFunctions->glVertex3f(vecRect.z, vecRect.y, origin.z);

		_openglFunctions->glVertex3f(vecRect.x, vecRect.w, origin.z);

		_openglFunctions->glVertex3f(vecRect.z, vecRect.w, origin.z);

		_openglFunctions->glEnd();
	}
}
}