#ifndef ENGINE_SHARED_SPRITE_CSPRITERENDERER_H
#define ENGINE_SHARED_SPRITE_CSPRITERENDERER_H

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "SpriteConstants.h"

class CSpriteEntity;

namespace sprite
{
struct msprite_t;

class CSpriteRenderer final
{
public:
	CSpriteRenderer();
	~CSpriteRenderer();

	void DrawSprite( CSpriteEntity* pEntity, const DrawFlags_t flags );

	void DrawSprite2D( const float flX, const float flY, const float flWidth, const float flHeight, const msprite_t* pSprite, const DrawFlags_t flags = DRAWF_NONE );

	void DrawSprite2D( const float flX, const float flY, const msprite_t* pSprite, const float flScale = 1, const DrawFlags_t flags = DRAWF_NONE );

private:

	void DrawSprite( const glm::vec3& vecOrigin, const glm::vec2& vecSize, const msprite_t* pSprite, const int iFrame, const DrawFlags_t flags );

private:
	CSpriteRenderer( const CSpriteRenderer& ) = delete;
	CSpriteRenderer& operator=( const CSpriteRenderer& ) = delete;
};

CSpriteRenderer& Renderer();
}

#endif //ENGINE_SHARED_SPRITE_CSPRITERENDERER_H