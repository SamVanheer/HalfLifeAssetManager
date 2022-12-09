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
	RenderPasses GetRenderPasses() const override { return RenderPass::Standard; }

	virtual void Draw(QOpenGLFunctions_1_1* openglFunctions, graphics::SceneContext& sc, RenderPasses renderPass) override;
};
