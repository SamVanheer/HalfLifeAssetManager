#include "engine/shared/studiomodel/EditableStudioModel.hpp"

#include "graphics/TextureLoader.hpp"

namespace studiomdl
{
Model* EditableStudioModel::GetModelByBodyPart(const int iBody, const int iBodyPart)
{
	auto& bodypart = *Bodyparts[iBodyPart];

	int index = iBody / bodypart.Base;
	index = index % static_cast<int>(bodypart.Models.size());

	return &bodypart.Models[index];
}

void EditableStudioModel::CreateTextures(graphics::TextureLoader& textureLoader)
{
	for (auto& texture : Textures)
	{
		GLuint name;

		glBindTexture(GL_TEXTURE_2D, 0);
		glGenTextures(1, &name);

		textureLoader.UploadIndexed8(
			name,
			texture->Width, texture->Height,
			texture->Pixels.data(),
			texture->Palette.data(),
			(texture->Flags & STUDIO_NF_NOMIPS) != 0,
			(texture->Flags & STUDIO_NF_MASKED) != 0);

		texture->TextureId = name;
	}
}
}
