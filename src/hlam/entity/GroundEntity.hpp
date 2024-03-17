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
	RenderPasses GetRenderPasses() const override { return RenderPass::Ground; }

	void Draw(graphics::SceneContext& sc, RenderPasses renderPass) override;

	void CreateDeviceObjects(graphics::SceneContext& sc) override;

	void DestroyDeviceObjects(graphics::SceneContext& sc) override;

	float GetRenderDistance(const glm::vec3& cameraOrigin) const override;

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
	bool EnableTexture = false;
	bool EnableGroundTextureTiling{false};
	int GroundTextureLength{16};

private:
	int _groundSequence{-1};
	float _previousGroundFrame{0};

	glm::vec2 _groundTextureOffset{0};

	GLuint _texture{0};
	std::string _imageName;
	graphics::Image _image;
	bool _hasTexture = false;
};
