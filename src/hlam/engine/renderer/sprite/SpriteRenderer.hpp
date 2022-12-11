#pragma once

#include <memory>

#include <spdlog/logger.h>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "engine/shared/renderer/DrawConstants.hpp"

#include "engine/shared/renderer/sprite/ISpriteRenderer.hpp"
#include "engine/shared/sprite/SpriteFileFormat.hpp"

class WorldTime;

namespace sprite
{
struct msprite_t;

class SpriteRenderer final : public ISpriteRenderer
{
private:
	static constexpr float DEFAULT_FRAMERATE{10};

public:
	SpriteRenderer(const std::shared_ptr<spdlog::logger>& logger, WorldTime* worldTime);
	~SpriteRenderer();

	SpriteRenderer(const SpriteRenderer&) = delete;
	SpriteRenderer& operator=(const SpriteRenderer&) = delete;

	void SetOpenGLFunctions(QOpenGLFunctions_1_1* openglFunctions) override
	{
		_openglFunctions = openglFunctions;
	}

	void DrawSprite(const SpriteRenderInfo& renderInfo, const renderer::DrawFlags flags) override;

	void DrawSprite2D(const float x, const float y, const float width, const float height,
		const msprite_t* sprite, const renderer::DrawFlags flags = renderer::DrawFlag::NONE) override;

	void DrawSprite2D(const float x, const float y, const msprite_t* sprite, const float scale = 1,
		const renderer::DrawFlags flags = renderer::DrawFlag::NONE) override;

	void DrawSprite2D(const Sprite2DRenderInfo& renderInfo, const renderer::DrawFlags flags = renderer::DrawFlag::NONE) override;

private:

	void DrawSprite(const glm::vec3& origin, const glm::vec2& size,
		const msprite_t* sprite, const float frameIndex,
		const renderer::DrawFlags flags, const sprite::Type::Type* typeOverride = nullptr, const sprite::TexFormat::TexFormat* texFormatOverride = nullptr);

private:
	std::shared_ptr<spdlog::logger> _logger;
	QOpenGLFunctions_1_1* _openglFunctions = nullptr;
	WorldTime* _worldTime;
};
}
