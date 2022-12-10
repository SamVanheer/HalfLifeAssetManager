#pragma once

#include <cstddef>

#include <qopenglfunctions_1_1.h>

#include "entity/BaseEntity.hpp"

#include "graphics/Image.hpp"

/**
*	Draws the current model texture.
*/
class TextureEntity : public BaseEntity
{
public:
	DECLARE_CLASS(TextureEntity, BaseEntity);

public:
	RenderPasses GetRenderPasses() const override { return RenderPass::Overlay2D; }

	void Draw(QOpenGLFunctions_1_1* openglFunctions, graphics::SceneContext& sc, RenderPasses renderPass) override;

	void CreateDeviceObjects(QOpenGLFunctions_1_1* openglFunctions, graphics::TextureLoader& textureLoader) override;

	void DestroyDeviceObjects(QOpenGLFunctions_1_1* openglFunctions, graphics::TextureLoader& textureLoader) override;

	void SetUVMeshImage(graphics::Image&& image)
	{
		_uvMeshImage = std::move(image);
	}

	int TextureIndex = -1;
	float TextureScale = 1;

	int XOffset = 0;
	int YOffset = 0;

	bool ShowUVMap = false;
	bool OverlayUVMap = false;

private:
	GLuint _uvMeshTexture = 0;
	graphics::Image _uvMeshImage;
};
