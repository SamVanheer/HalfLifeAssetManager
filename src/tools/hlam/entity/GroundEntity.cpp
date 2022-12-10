#include "entity/GroundEntity.hpp"
#include "entity/HLMVStudioModelEntity.hpp"

#include "graphics/GraphicsUtils.hpp"

#include "ui/assets/studiomodel/StudioModelAsset.hpp"
#include "ui/assets/studiomodel/StudioModelColors.hpp"
#include "ui/EditorContext.hpp"
#include "ui/settings/ColorSettings.hpp"
#include "ui/settings/StudioModelSettings.hpp"

void GroundEntity::Draw(QOpenGLFunctions_1_1* openglFunctions, graphics::SceneContext& sc, RenderPasses renderPass)
{
	auto context = GetContext();
	auto settings = context->Settings;

	if (settings->ShowGround)
	{
		// Update image if changed.
		if (!_image.GetData().empty())
		{
			openglFunctions->glBindTexture(GL_TEXTURE_2D, _texture);

			openglFunctions->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
				_image.GetWidth(), _image.GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, _image.GetData().data());
			openglFunctions->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			openglFunctions->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

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

		const float floorTextureLength = settings->EnableFloorTextureTiling ? settings->FloorTextureLength : settings->GetFloorLength();

		//Prevent the offset from overflowing
		_floorTextureOffset.x = std::fmod(_floorTextureOffset.x, floorTextureLength);
		_floorTextureOffset.y = std::fmod(_floorTextureOffset.y, floorTextureLength);

		auto colors = GetContext()->Asset->GetEditorContext()->GetColorSettings();

		graphics::DrawFloor(openglFunctions,
			settings->FloorOrigin, settings->GetFloorLength(), floorTextureLength, _floorTextureOffset, _texture,
			ui::assets::studiomodel::ColorToVector(colors->GetColor(ui::assets::studiomodel::GroundColor.Name)), settings->MirrorOnGround);
	}
}

void GroundEntity::CreateDeviceObjects(QOpenGLFunctions_1_1* openglFunctions, graphics::TextureLoader& textureLoader)
{
	openglFunctions->glGenTextures(1, &_texture);
}

void GroundEntity::DestroyDeviceObjects(QOpenGLFunctions_1_1* openglFunctions, graphics::TextureLoader& textureLoader)
{
	openglFunctions->glDeleteTextures(1, &_texture);
	_texture = 0;
}
