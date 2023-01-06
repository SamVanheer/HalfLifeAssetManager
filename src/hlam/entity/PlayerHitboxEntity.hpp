#pragma once

#include "entity/BaseEntity.hpp"

/**
*	Draws a box representing the in-game player hitbox.
*/
class PlayerHitboxEntity : public BaseEntity
{
public:
	DECLARE_CLASS(PlayerHitboxEntity, BaseEntity);

public:
	RenderPasses GetRenderPasses() const override { return RenderPass::Overlay3D; }

	virtual void Draw(graphics::SceneContext& sc, RenderPasses renderPass) override;

	bool ShowPlayerHitbox = false;
};
