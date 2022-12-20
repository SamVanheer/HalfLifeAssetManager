#pragma once

#include <string>

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

	const std::string& GetImageName() const { return _imageName; }

	void SetImage(std::string&& imageName, graphics::Image&& image)
	{
		_imageName = std::move(imageName);
		_image = std::move(image);
	}

	bool ShowBackground = false;

private:
	GLuint _texture{0};
	std::string _imageName;
	graphics::Image _image;
};
