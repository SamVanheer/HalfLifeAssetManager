#pragma once

#include "entity/BaseEntity.hpp"

/**
*	Draws axes at the world origin.
*/
class AxesEntity : public BaseEntity
{
public:
	DECLARE_CLASS(AxesEntity, BaseEntity);

public:
	RenderPasses GetRenderPasses() const override { return RenderPass::Overlay3D; }

	virtual void Draw(graphics::SceneContext& sc, RenderPasses renderPass) override;

	bool ShowAxes = false;
};
