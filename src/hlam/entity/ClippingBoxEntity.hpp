#pragma once

#include "entity/BaseEntity.hpp"

/**
*	Draws the model's clipping box.
*/
class ClippingBoxEntity : public BaseEntity
{
public:
	DECLARE_CLASS(ClippingBoxEntity, BaseEntity);

public:
	RenderPasses GetRenderPasses() const override { return RenderPass::Overlay3D; }

	virtual void Draw(graphics::SceneContext& sc, RenderPasses renderPass) override;

	bool ShowCBox = false;
};
