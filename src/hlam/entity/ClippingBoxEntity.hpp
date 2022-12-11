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
	RenderPasses GetRenderPasses() const override { return RenderPass::Standard; }

	virtual void Draw(QOpenGLFunctions_1_1* openglFunctions, graphics::SceneContext& sc, RenderPasses renderPass) override;
};
