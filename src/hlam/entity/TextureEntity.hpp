#pragma once

#include <cstddef>
#include <vector>

#include "entity/BaseEntity.hpp"

namespace studiomdl
{
struct StudioMesh;
}

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

	void SetTextureIndex(int textureIndex, int meshIndex);

	void SetMeshIndex(int meshIndex);

	float TextureScale = 1;

	int XOffset = 0;
	int YOffset = 0;

	bool ShowUVMap = false;
	bool OverlayUVMap = false;
	bool AntiAliasLines = false;

private:
	int _textureIndex = -1;
	std::vector<const studiomdl::StudioMesh*> _meshes;
};
