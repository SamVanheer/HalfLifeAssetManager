#pragma once

#include "entity/BaseAnimating.hpp"

namespace sprite
{
struct msprite_t;
}

class SpriteEntity : public BaseAnimating
{
public:
	DECLARE_CLASS(SpriteEntity, BaseAnimating);

	~SpriteEntity();

	virtual void Spawn() override;

	virtual void Draw(renderer::DrawFlags flags) override;

	void AnimThink();

	sprite::msprite_t* GetSprite() const { return _sprite; }

	void SetSprite(sprite::msprite_t* pSprite);

private:
	sprite::msprite_t* _sprite = nullptr;
};
