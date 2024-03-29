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

	RenderPasses GetRenderPasses() const override { return RenderPass::Standard; }

	virtual void Draw(graphics::SceneContext& sc, RenderPasses renderPass) override;

	void AnimThink();

	sprite::msprite_t* GetSprite() const { return _sprite; }

	void SetSprite(sprite::msprite_t* pSprite);

private:
	sprite::msprite_t* _sprite = nullptr;
};
