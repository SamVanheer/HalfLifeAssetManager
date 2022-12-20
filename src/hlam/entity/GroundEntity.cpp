#include "entity/GroundEntity.hpp"
#include "entity/HLMVStudioModelEntity.hpp"

#include "graphics/GraphicsUtils.hpp"
#include "graphics/SceneContext.hpp"

#include "ui/assets/studiomodel/StudioModelAsset.hpp"
#include "ui/assets/studiomodel/StudioModelColors.hpp"
#include "ui/EditorContext.hpp"
#include "ui/settings/ColorSettings.hpp"
#include "ui/settings/StudioModelSettings.hpp"

void GroundEntity::Draw(graphics::SceneContext& sc, RenderPasses renderPass)
{
	auto context = GetContext();
	auto settings = context->Settings;

	if (ShowGround)
	{
		// Update image if changed.
		if (!_image.GetData().empty())
		{
			sc.OpenGLFunctions->glBindTexture(GL_TEXTURE_2D, _texture);

			sc.OpenGLFunctions->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
				_image.GetWidth(), _image.GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, _image.GetData().data());
			sc.OpenGLFunctions->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			sc.OpenGLFunctions->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			_image = {};
		}

		glm::vec2 textureOffset{0};

		//Calculate texture offset based on sequence movement and current frame
		if (auto entity = context->Asset->GetEntity(); entity)
		{
			auto model = entity->GetEditableModel();

			const int sequenceIndex = entity->GetSequence();

			if (sequenceIndex >= 0 && sequenceIndex < model->Sequences.size())
			{
				const auto& sequence = *model->Sequences[entity->GetSequence()];

				//Scale offset to current frame
				const float currentFrame = entity->GetFrame() / (sequence.NumFrames - 1);

				float delta;

				if (currentFrame >= _previousFloorFrame)
				{
					delta = currentFrame - _previousFloorFrame;
				}
				else
				{
					delta = (currentFrame + 1) - _previousFloorFrame;
				}

				_previousFloorFrame = currentFrame;

				//Adjust scrolling direction based on whether the model is mirrored, but don't apply scale itself
				const int xDirection = entity->GetScale().x > 0 ? 1 : -1;
				const int yDirection = entity->GetScale().y > 0 ? 1 : -1;

				textureOffset.x = sequence.LinearMovement.x * delta * xDirection;
				textureOffset.y = -(sequence.LinearMovement.y * delta * yDirection);

				if (_floorSequence != entity->GetSequence())
				{
					_floorSequence = entity->GetSequence();
					_previousFloorFrame = 0;
					_floorTextureOffset.x = _floorTextureOffset.y = 0;
				}
			}
		}

		_floorTextureOffset += textureOffset;

		const float floorTextureLength = EnableFloorTextureTiling ? FloorTextureLength : settings->GetFloorLength();

		//Prevent the offset from overflowing
		_floorTextureOffset.x = std::fmod(_floorTextureOffset.x, floorTextureLength);
		_floorTextureOffset.y = std::fmod(_floorTextureOffset.y, floorTextureLength);

		auto colors = GetContext()->Asset->GetEditorContext()->GetColorSettings();

		std::optional<GLuint> texture;

		if (_hasTexture)
		{
			texture = _texture;
		}

		graphics::DrawFloor(sc.OpenGLFunctions,
			GetOrigin(), settings->GetFloorLength(), floorTextureLength, _floorTextureOffset, texture,
			colors->GetColor(studiomodel::GroundColor.Name), MirrorOnGround);
	}
}

void GroundEntity::CreateDeviceObjects(graphics::SceneContext& sc)
{
	sc.OpenGLFunctions->glGenTextures(1, &_texture);
}

void GroundEntity::DestroyDeviceObjects(graphics::SceneContext& sc)
{
	sc.OpenGLFunctions->glDeleteTextures(1, &_texture);
	_texture = 0;
}
