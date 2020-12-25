#pragma once

#include "CBaseAnimating.hpp"

namespace sprite
{
struct msprite_t;
}

class CSpriteEntity : public CBaseAnimating
{
public:
	DECLARE_CLASS( CSpriteEntity, CBaseAnimating );

	virtual void OnDestroy() override;

	virtual bool Spawn() override;

	virtual void Draw( renderer::DrawFlags_t flags ) override;

	void AnimThink();

	sprite::msprite_t* GetSprite() const { return m_pSprite; }

	void SetSprite( sprite::msprite_t* pSprite );

private:
	sprite::msprite_t* m_pSprite = nullptr;
};
