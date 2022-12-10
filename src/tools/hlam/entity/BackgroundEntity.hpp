#pragma once

#include <qopenglfunctions_1_1.h>

#include "entity/BaseEntity.hpp"

#include "graphics/Image.hpp"

/**
*	Draws the background.
*/
class BackgroundEntity : public BaseEntity
{
public:
	DECLARE_CLASS(BackgroundEntity, BaseEntity);

public:
	RenderPasses GetRenderPasses() const override { return RenderPass::Background; }

	void Draw(QOpenGLFunctions_1_1* openglFunctions, graphics::SceneContext& sc, RenderPasses renderPass) override;

	void CreateDeviceObjects(QOpenGLFunctions_1_1* openglFunctions, graphics::TextureLoader& textureLoader) override;

	void DestroyDeviceObjects(QOpenGLFunctions_1_1* openglFunctions, graphics::TextureLoader& textureLoader) override;

	void SetImage(graphics::Image&& image)
	{
		_image = std::move(image);
	}

private:
	GLuint _texture{0};
	graphics::Image _image;
};
