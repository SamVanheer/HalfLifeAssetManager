#pragma once

#include "entity/BaseEntity.hpp"

/**
*	Draws the crosshair.
*/
class GuidelinesEntity : public BaseEntity
{
public:
	DECLARE_CLASS(GuidelinesEntity, BaseEntity);

public:
	static constexpr int GUIDELINES_LINE_WIDTH = 1;
	static constexpr int GUIDELINES_LINE_LENGTH = 5;
	static constexpr int GUIDELINES_POINT_LINE_OFFSET = 2;
	static constexpr int GUIDELINES_OFFSET = GUIDELINES_LINE_LENGTH + (GUIDELINES_POINT_LINE_OFFSET * 2) + GUIDELINES_LINE_WIDTH;

	static constexpr int GUIDELINES_EDGE_WIDTH = 4;

	RenderPasses GetRenderPasses() const override { return RenderPass::Overlay2D; }

	virtual void Draw(graphics::SceneContext& sc, RenderPasses renderPass) override;

	bool ShowGuidelines = false;
	bool ShowOffscreenAreas = true;
};
