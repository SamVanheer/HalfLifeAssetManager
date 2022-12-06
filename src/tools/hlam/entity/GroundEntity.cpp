#include "entity/GroundEntity.hpp"
#include "entity/HLMVStudioModelEntity.hpp"

#include "graphics/GraphicsUtils.hpp"
#include "graphics/Scene.hpp"

#include "ui/settings/StudioModelSettings.hpp"

void GroundEntity::Draw(QOpenGLFunctions_1_1* openglFunctions, RenderPasses renderPass)
{
	auto context = GetContext();
	auto settings = context->Settings;

	if (settings->ShowGround)
	{
		glm::vec2 textureOffset{0};

		//Calculate texture offset based on sequence movement and current frame
		if (auto entity = context->Scene->GetEntity(); entity)
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

		graphics::DrawFloor(openglFunctions, settings->FloorOrigin, settings->GetFloorLength(), floorTextureLength, _floorTextureOffset, context->Scene->GroundTexture, context->Scene->GroundColor, settings->MirrorOnGround);
	}
}
