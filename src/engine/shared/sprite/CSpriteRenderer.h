#ifndef ENGINE_SHARED_SPRITE_CSPRITERENDERER_H
#define ENGINE_SHARED_SPRITE_CSPRITERENDERER_H

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "engine/shared/renderer/DrawConstants.h"

class CSpriteEntity;

namespace sprite
{
struct msprite_t;

class CSpriteRenderer final
{
public:
	CSpriteRenderer();
	~CSpriteRenderer();

	void DrawSprite( CSpriteEntity* pEntity, const renderer::DrawFlags_t flags );

	void DrawSprite2D( const float flX, const float flY, const float flWidth, const float flHeight, const msprite_t* pSprite, const renderer::DrawFlags_t flags = renderer::DrawFlag::NONE );

	void DrawSprite2D( const float flX, const float flY, const msprite_t* pSprite, const float flScale = 1, const renderer::DrawFlags_t flags = renderer::DrawFlag::NONE );

private:

	void DrawSprite( const glm::vec3& vecOrigin, const glm::vec2& vecSize, const msprite_t* pSprite, const int iFrame, const renderer::DrawFlags_t flags );

private:
	CSpriteRenderer( const CSpriteRenderer& ) = delete;
	CSpriteRenderer& operator=( const CSpriteRenderer& ) = delete;
};

CSpriteRenderer& Renderer();
}

#endif //ENGINE_SHARED_SPRITE_CSPRITERENDERER_H