#pragma once

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

	void Draw(graphics::SceneContext& sc, RenderPasses renderPass) override;

	void CreateDeviceObjects(graphics::SceneContext& sc) override;

	void DestroyDeviceObjects(graphics::SceneContext& sc) override;

	void SetImage(graphics::Image&& image)
	{
		_image = std::move(image);
	}

private:
	GLuint _texture{0};
	graphics::Image _image;
};
