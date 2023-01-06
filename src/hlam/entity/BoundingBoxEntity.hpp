#pragma once

#include "entity/BaseEntity.hpp"

/**
*	Draws the model's bounding box.
*/
class BoundingBoxEntity : public BaseEntity
{
public:
	DECLARE_CLASS(BoundingBoxEntity, BaseEntity);

public:
	RenderPasses GetRenderPasses() const override { return RenderPass::Overlay3D; }

	virtual void Draw(graphics::SceneContext& sc, RenderPasses renderPass) override;

	bool ShowBBox = false;
};
