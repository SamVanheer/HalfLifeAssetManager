#pragma once

#include "entity/BaseEntity.hpp"

/**
*	Draws the crosshair.
*/
class CrosshairEntity : public BaseEntity
{
public:
	DECLARE_CLASS(CrosshairEntity, BaseEntity);

public:
	static constexpr int CROSSHAIR_LINE_WIDTH = 3;
	static constexpr int CROSSHAIR_LINE_START = 5;
	static constexpr int CROSSHAIR_LINE_LENGTH = 10;
	static constexpr int CROSSHAIR_LINE_END = CROSSHAIR_LINE_START + CROSSHAIR_LINE_LENGTH;

	RenderPasses GetRenderPasses() const override { return RenderPass::Overlay2D; }

	virtual void Draw(graphics::SceneContext& sc, RenderPasses renderPass) override;
};
