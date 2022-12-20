#pragma once

#include <cstddef>
#include <string>

#include <glm/vec2.hpp>

#include "entity/BaseEntity.hpp"

#include "graphics/Image.hpp"

/**
*	Draws the ground.
*/
class GroundEntity : public BaseEntity
{
public:
	DECLARE_CLASS(GroundEntity, BaseEntity);

public:
	RenderPasses GetRenderPasses() const override { return RenderPass::Standard; }

	void Draw(graphics::SceneContext& sc, RenderPasses renderPass) override;

	void CreateDeviceObjects(graphics::SceneContext& sc) override;

	void DestroyDeviceObjects(graphics::SceneContext& sc) override;

	const std::string& GetImageName() const { return _imageName; }

	void SetImage(std::string&& imageName, graphics::Image&& image)
	{
		_imageName = std::move(imageName);
		_image = std::move(image);
		_hasTexture = true;
	}

	void ClearImage()
	{
		_hasTexture = false;
	}

	bool ShowGround = false;
	bool MirrorOnGround = false;
	bool EnableFloorTextureTiling{false};
	int FloorTextureLength{16};

private:
	int _floorSequence{-1};
	float _previousFloorFrame{0};

	glm::vec2 _floorTextureOffset{0};

	GLuint _texture{0};
	std::string _imageName;
	graphics::Image _image;
	bool _hasTexture = false;
};
