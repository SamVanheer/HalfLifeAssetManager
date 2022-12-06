#pragma once

#include "entity/BaseEntity.hpp"

/**
*	Draws the model's clipping box.
*/
class BackgroundEntity : public BaseEntity
{
public:
	DECLARE_CLASS(BackgroundEntity, BaseEntity);

public:
	RenderPasses GetRenderPasses() const override { return RenderPass::Background; }

	virtual void Draw(QOpenGLFunctions_1_1* openglFunctions, RenderPasses renderPass) override;
};
