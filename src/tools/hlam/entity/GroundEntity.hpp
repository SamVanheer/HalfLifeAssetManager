#pragma once

#include <cstddef>

#include <qopenglfunctions_1_1.h>

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

	void Draw(QOpenGLFunctions_1_1* openglFunctions, graphics::SceneContext& sc, RenderPasses renderPass) override;

	void CreateDeviceObjects(QOpenGLFunctions_1_1* openglFunctions, graphics::TextureLoader& textureLoader) override;

	void DestroyDeviceObjects(QOpenGLFunctions_1_1* openglFunctions, graphics::TextureLoader& textureLoader) override;

	void SetImage(QOpenGLFunctions_1_1* openglFunctions, int width, int height, const std::byte* data);

private:
	int _floorSequence{-1};
	float _previousFloorFrame{0};

	glm::vec2 _floorTextureOffset{0};

	GLuint _texture{0};
};
