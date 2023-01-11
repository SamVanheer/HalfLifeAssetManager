#include <QOpenGLFunctions_1_1>

#include <glm/geometric.hpp>

#include "entity/GroundEntity.hpp"
#include "entity/HLMVStudioModelEntity.hpp"

#include "graphics/GraphicsUtils.hpp"
#include "graphics/SceneContext.hpp"

#include "settings/ColorSettings.hpp"

#include "plugins/halflife/studiomodel/StudioModelAsset.hpp"
#include "plugins/halflife/studiomodel/StudioModelColors.hpp"
#include "plugins/halflife/studiomodel/settings/StudioModelSettings.hpp"
#include "application/AssetManager.hpp"

void GroundEntity::Draw(graphics::SceneContext& sc, RenderPasses renderPass)
{
	auto context = GetContext();
	auto settings = context->StudioSettings;

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

				if (currentFrame >= _previousGroundFrame)
				{
					delta = currentFrame - _previousGroundFrame;
				}
				else
				{
					delta = (currentFrame + 1) - _previousGroundFrame;
				}

				_previousGroundFrame = currentFrame;

				//Adjust scrolling direction based on whether the model is mirrored, but don't apply scale itself
				const int xDirection = entity->GetScale().x > 0 ? 1 : -1;
				const int yDirection = entity->GetScale().y > 0 ? 1 : -1;

				textureOffset.x = sequence.LinearMovement.x * delta * xDirection;
				textureOffset.y = -(sequence.LinearMovement.y * delta * yDirection);

				if (_groundSequence != entity->GetSequence())
				{
					_groundSequence = entity->GetSequence();
					_previousGroundFrame = 0;
					_groundTextureOffset.x = _groundTextureOffset.y = 0;
				}
			}
		}

		_groundTextureOffset += textureOffset;

		const float groundTextureLength = EnableGroundTextureTiling ? GroundTextureLength : settings->GetGroundLength();

		//Prevent the offset from overflowing
		_groundTextureOffset.x = std::fmod(_groundTextureOffset.x, groundTextureLength);
		_groundTextureOffset.y = std::fmod(_groundTextureOffset.y, groundTextureLength);

		auto colors = GetContext()->Asset->GetApplication()->GetColorSettings();

		std::optional<GLuint> texture;

		if (EnableTexture)
		{
			texture = _hasTexture ? _texture : GetContext()->Asset->GetProvider()->GetDefaultGroundTexture();
		}

		graphics::DrawGround(sc.OpenGLFunctions,
			GetOrigin(), settings->GetGroundLength(), groundTextureLength, _groundTextureOffset, texture,
			colors->GetColor(studiomodel::GroundColor), MirrorOnGround);
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

float GroundEntity::GetRenderDistance(const glm::vec3& cameraOrigin) const
{
	return glm::length(GetOrigin() - cameraOrigin);
}
