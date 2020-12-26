#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "engine/shared/renderer/DrawConstants.hpp"

#include "engine/shared/renderer/sprite/ISpriteRenderer.hpp"
#include "engine/shared/sprite/sprite.hpp"

class WorldTime;

namespace sprite
{
struct msprite_t;

class SpriteRenderer final : public ISpriteRenderer
{
private:
	static constexpr float DEFAULT_FRAMERATE{10};

public:
	SpriteRenderer(WorldTime* worldTime);
	~SpriteRenderer();

	SpriteRenderer(const SpriteRenderer&) = delete;
	SpriteRenderer& operator=(const SpriteRenderer&) = delete;

	void DrawSprite(const SpriteRenderInfo* renderInfo, const renderer::DrawFlags_t flags) override;

	void DrawSprite2D(const float x, const float y, const float width, const float height,
		const msprite_t* sprite, const renderer::DrawFlags_t flags = renderer::DrawFlag::NONE) override;

	void DrawSprite2D(const float x, const float y, const msprite_t* sprite, const float scale = 1,
		const renderer::DrawFlags_t flags = renderer::DrawFlag::NONE) override;

	void DrawSprite2D(const Sprite2DRenderInfo* renderInfo, const renderer::DrawFlags_t flags = renderer::DrawFlag::NONE) override;

private:

	void DrawSprite(const glm::vec3& origin, const glm::vec2& size,
		const msprite_t* sprite, const float frameIndex,
		const renderer::DrawFlags_t flags, const sprite::Type::Type* typeOverride = nullptr, const sprite::TexFormat::TexFormat* texFormatOverride = nullptr);

private:
	WorldTime* _worldTime;
};
}
