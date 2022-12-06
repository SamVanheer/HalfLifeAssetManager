#pragma once

#include <glm/vec2.hpp>

#include "entity/BaseEntity.hpp"

/**
*	Draws the ground.
*/
class GroundEntity : public BaseEntity
{
public:
	DECLARE_CLASS(GroundEntity, BaseEntity);

public:
	RenderPasses GetRenderPasses() const override { return RenderPass::Standard; }

	virtual void Draw(QOpenGLFunctions_1_1* openglFunctions, RenderPasses renderPass) override;

private:
	int _floorSequence{-1};
	float _previousFloorFrame{0};

	glm::vec2 _floorTextureOffset{0};
};
