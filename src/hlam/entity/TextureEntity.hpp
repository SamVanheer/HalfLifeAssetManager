#pragma once

#include <cstddef>

#include "entity/BaseEntity.hpp"

/**
*	Draws the current model texture.
*/
class TextureEntity : public BaseEntity
{
public:
	DECLARE_CLASS(TextureEntity, BaseEntity);

public:
	RenderPasses GetRenderPasses() const override { return RenderPass::Overlay2D; }

	void Draw(graphics::SceneContext& sc, RenderPasses renderPass) override;

	int TextureIndex = -1;
	float TextureScale = 1;
	int MeshIndex = -1;

	int XOffset = 0;
	int YOffset = 0;

	bool ShowUVMap = false;
	bool OverlayUVMap = false;
	bool AntiAliasLines = false;
};
