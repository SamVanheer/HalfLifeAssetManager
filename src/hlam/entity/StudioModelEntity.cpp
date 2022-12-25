#include <algorithm>

#include <glm/geometric.hpp>

#include "entity/GroundEntity.hpp"
#include "entity/StudioModelEntity.hpp"

#include "graphics/GraphicsUtils.hpp"
#include "graphics/SceneContext.hpp"

#include "ui/assets/studiomodel/StudioModelAsset.hpp"
#include "ui/settings/StudioModelSettings.hpp"

#include "utility/WorldTime.hpp"

StudioModelEntity::StudioModelEntity(studiomdl::EditableStudioModel* model)
{
	SetEditableModel(model);
}

void StudioModelEntity::Spawn()
{
	SetSequence(0);
	SetController(0, 0.0f);
	SetController(1, 0.0f);
	SetController(2, 0.0f);
	SetController(3, 0.0f);
	SetMouth(0.0f);

	for (int n = 0; n < _editableModel->Bodyparts.size(); ++n)
	{
		SetBodygroup(n, 0);
	}

	SetSkin(0);
}

void StudioModelEntity::Draw(graphics::SceneContext& sc, RenderPasses renderPass)
{
	auto asset = GetContext()->Asset;
	auto settings = GetContext()->Settings;

	// setup stencil buffer and draw mirror
	if (asset->GetGroundEntity()->MirrorOnGround)
	{
		graphics::DrawMirroredModel(sc.OpenGLFunctions,
			*GetContext()->StudioModelRenderer, this,
			asset->CurrentRenderMode,
			asset->ShowWireframeOverlay,
			asset->GetGroundEntity()->GetOrigin(),
			settings->GetFloorLength(),
			asset->EnableBackfaceCulling);
	}

	graphics::SetupRenderMode(sc.OpenGLFunctions, asset->CurrentRenderMode, asset->EnableBackfaceCulling);

	const glm::vec3& vecScale = GetScale();

	//Determine if an odd number of scale values are negative. The cull face has to be changed if so.
	const float flScale = vecScale.x * vecScale.y * vecScale.z;

	sc.OpenGLFunctions->glCullFace(flScale > 0 ? GL_FRONT : GL_BACK);

	renderer::DrawFlags flags = renderer::DrawFlag::NONE;

	if (asset->ShowWireframeOverlay)
	{
		flags |= renderer::DrawFlag::WIREFRAME_OVERLAY;
	}

	if (asset->CameraIsFirstPerson())
	{
		flags |= renderer::DrawFlag::IS_VIEW_MODEL;
	}

	if (asset->DrawShadows)
	{
		flags |= renderer::DrawFlag::DRAW_SHADOWS;
	}

	if (asset->FixShadowZFighting)
	{
		flags |= renderer::DrawFlag::FIX_SHADOW_Z_FIGHTING;
	}

	//TODO: these should probably be made separate somehow
	if (asset->ShowHitboxes)
	{
		flags |= renderer::DrawFlag::DRAW_HITBOXES;
	}

	if (asset->ShowBones)
	{
		flags |= renderer::DrawFlag::DRAW_BONES;
	}

	if (asset->ShowAttachments)
	{
		flags |= renderer::DrawFlag::DRAW_ATTACHMENTS;
	}

	if (asset->ShowEyePosition)
	{
		flags |= renderer::DrawFlag::DRAW_EYE_POSITION;
	}

	if (asset->ShowNormals)
	{
		flags |= renderer::DrawFlag::DRAW_NORMALS;
	}

	studiomdl::ModelRenderInfo renderInfo = GetRenderInfo();

	GetContext()->StudioModelRenderer->DrawModel(renderInfo, flags);

	//TODO: this is a temporary hack. The graphics scene architecture needs a complete overhaul first,
		//then this can be done by rendering the model in a separate viewmodel layer
	if (asset->CameraIsFirstPerson())
	{
		renderInfo.Origin.z -= 1;
	}

	if (asset->DrawSingleBoneIndex != -1)
	{
		GetContext()->StudioModelRenderer->DrawSingleBone(renderInfo, asset->DrawSingleBoneIndex);
	}

	if (asset->DrawSingleAttachmentIndex != -1)
	{
		GetContext()->StudioModelRenderer->DrawSingleAttachment(renderInfo, asset->DrawSingleAttachmentIndex);
	}

	if (asset->DrawSingleHitboxIndex != -1)
	{
		GetContext()->StudioModelRenderer->DrawSingleHitbox(renderInfo, asset->DrawSingleHitboxIndex);
	}
}

void StudioModelEntity::CreateDeviceObjects(graphics::SceneContext& sc)
{
	auto model = GetEditableModel();

	model->CreateTextures(*sc.TextureLoader);
}

void StudioModelEntity::DestroyDeviceObjects(graphics::SceneContext& sc)
{
	auto model = GetEditableModel();

	model->DeleteTextures(*sc.TextureLoader);
}

float StudioModelEntity::GetRenderDistance(const glm::vec3& cameraOrigin) const
{
	// Models should use the model's center point for rendering.
	glm::vec3 mins, maxs;
	ExtractBbox(mins, maxs);

	const glm::vec3 modelCenter = ((mins + maxs) * 0.5f);
	const glm::vec3 renderOrigin = GetOrigin() + modelCenter;

	return glm::length(renderOrigin - cameraOrigin);
}

studiomdl::ModelRenderInfo StudioModelEntity::GetRenderInfo() const
{
	studiomdl::ModelRenderInfo renderInfo{};

	renderInfo.Origin = GetOrigin();
	renderInfo.Angles = GetAngles();
	renderInfo.Scale = GetScale();

	renderInfo.Model = GetEditableModel();

	renderInfo.Transparency = GetTransparency();
	renderInfo.Sequence = GetSequence();
	renderInfo.Frame = GetFrame();
	renderInfo.Bodygroup = GetBodygroup();
	renderInfo.Skin = GetSkin();

	for (int i = 0; i < STUDIO_MAX_BLENDERS; ++i)
	{
		renderInfo.Blender[i] = GetBlendingByIndex(i);
	}

	for (int i = 0; i < STUDIO_MAX_CONTROLLERS; ++i)
	{
		renderInfo.Controller[i] = GetControllerByIndex(i);
	}

	renderInfo.Mouth = GetMouth();

	return renderInfo;
}

float StudioModelEntity::AdvanceFrame(float deltaTime, const float maximum)
{
	if (!_editableModel)
	{
		return 0;
	}

	if (_sequence < 0 || _sequence >= _editableModel->Sequences.size())
	{
		return 0;
	}

	deltaTime = std::max(0.f, deltaTime);

	const auto& sequenceDescriptor = *_editableModel->Sequences[_sequence];

	if (deltaTime == 0)
	{
		deltaTime = (GetContext()->Time->GetTime() - _animTime);

		if (deltaTime <= 0.001)
		{
			return 0;
		}
	}

	if (!_animTime)
	{
		deltaTime = 0;
	}

	if (maximum != -1.f)
	{
		deltaTime = std::min(deltaTime, maximum);
	}

	const float oldFrame = _frame;

	bool shouldLoop;

	switch (_loopingMode)
	{
	default:
	case StudioLoopingMode::AlwaysLoop:
		shouldLoop = true;
		break;
	case StudioLoopingMode::NeverLoop:
		shouldLoop = false;
		break;
	case StudioLoopingMode::UseSequenceSetting:
		shouldLoop = (sequenceDescriptor.Flags & STUDIO_LOOPING) != 0;
		break;
	}

	const float increment = deltaTime * sequenceDescriptor.FPS * _frameRate;

	if (_frame < (sequenceDescriptor.NumFrames - 1) || shouldLoop)
	{
		_frame += increment;
	}

	if (sequenceDescriptor.NumFrames <= 1)
	{
		_frame = 0;
	}
	else
	{
		if (shouldLoop)
		{
			// wrap
			_frame -= (int)(_frame / (sequenceDescriptor.NumFrames - 1)) * (sequenceDescriptor.NumFrames - 1);
		}
		else if (_frame >= (sequenceDescriptor.NumFrames - 1))
		{
			//Clamp frame to the last valid frame index
			_frame = static_cast<float>(sequenceDescriptor.NumFrames - 1);
		}

		//Wrapped
		if (oldFrame > _frame)
		{
			_lastEventCheck = _frame - increment;
		}
	}

	_animTime = GetContext()->Time->GetTime();

	return deltaTime;
}

int StudioModelEntity::GetAnimationEvent(AnimEvent& event, float start, float end, int index)
{
	if (!_editableModel)
	{
		return 0;
	}

	if (_sequence < 0 || _sequence >= _editableModel->Sequences.size())
	{
		return 0;
	}

	int events = 0;

	const auto& sequenceDescriptor = *_editableModel->Sequences[_sequence];

	if (index >= sequenceDescriptor.SortedEvents.size())
	{
		return 0;
	}

	if (sequenceDescriptor.NumFrames <= 1)
	{
		start = 0;
		end = 1.0;
	}

	for (; index < sequenceDescriptor.SortedEvents.size(); index++)
	{
		const auto& candidate = *sequenceDescriptor.SortedEvents[index];

		if ((candidate.Frame >= start && candidate.Frame < end) ||
			((sequenceDescriptor.Flags & STUDIO_LOOPING)
				&& end >= sequenceDescriptor.NumFrames - 1
				&& candidate.Frame < end - sequenceDescriptor.NumFrames + 1))
		{
			event.id = candidate.EventId;
			event.options = candidate.Options.data();
			return index + 1;
		}
	}

	return 0;
}

void StudioModelEntity::DispatchAnimEvents()
{
	if (!_editableModel)
	{
		return;
	}

	// This is based on Source's DispatchAnimEvents.
	// It fixes the bug where events don't get triggered, and get triggered multiple times due to the workaround.
	// Plays from previous frame to current.
	// This differs from GoldSource in that GoldSource plays from current to predicted future frame.
	// This is more accurate, since it's based on actual frame data, rather than predicted frames,
	// but results in events firing later than before.
	// The difference is ~0.1 seconds when running at 60 FPS.
	// On dedicated servers the difference will be smaller if the tick rate is higher.
	float start = _lastEventCheck;
	float end = _frame;
	_lastEventCheck = _frame;

	AnimEvent event;

	int index = 0;

	while ((index = GetAnimationEvent(event, start, end, index)) != 0)
	{
		HandleAnimEvent(event);
	}
}

void StudioModelEntity::HandleAnimEvent(const AnimEvent& event)
{
}

void StudioModelEntity::SetFrame(float frame)
{
	if (frame == -1)
	{
		return;
	}

	if (!_editableModel)
	{
		return;
	}

	if (_sequence < 0 || _sequence >= _editableModel->Sequences.size())
	{
		return;
	}

	const auto& sequenceDescriptor = *_editableModel->Sequences[_sequence];

	_frame = frame;

	if (sequenceDescriptor.NumFrames <= 1)
	{
		_frame = 0;
	}
	else
	{
		// wrap
		_frame -= (int)(_frame / (sequenceDescriptor.NumFrames - 1)) * (sequenceDescriptor.NumFrames - 1);
	}

	_animTime = GetContext()->Time->GetTime();
}

void StudioModelEntity::SetEditableModel(studiomdl::EditableStudioModel* model)
{
	_editableModel = model;
}

int StudioModelEntity::GetNumFrames() const
{
	if (_sequence < 0 || _sequence >= _editableModel->Sequences.size())
	{
		return 0;
	}

	const auto& sequenceDescriptor = *_editableModel->Sequences[_sequence];

	return sequenceDescriptor.NumFrames;
}

void StudioModelEntity::SetSequence(int sequence)
{
	//index of -1 means no sequence
	if (_editableModel->Sequences.empty())
	{
		sequence = -1;
	}
	else
	{
		if (sequence != -1 && (sequence < -1 || sequence >= _editableModel->Sequences.size()))
		{
			return;
		}
	}

	_sequence = sequence;
	_frame = 0;
	_lastEventCheck = 0;
}

void StudioModelEntity::GetSequenceInfo(float& frameRate, float& groundSpeed) const
{
	bool hasSequenceInfo = false;

	if (_sequence != -1)
	{
		const auto& sequenceDescriptor = *_editableModel->Sequences[_sequence];

		if (sequenceDescriptor.NumFrames > 1)
		{
			frameRate = sequenceDescriptor.FPS;
			groundSpeed = static_cast<float>(glm::length(sequenceDescriptor.LinearMovement));
			groundSpeed = groundSpeed * sequenceDescriptor.FPS / (sequenceDescriptor.NumFrames - 1);

			hasSequenceInfo = true;
		}
	}

	if (!hasSequenceInfo)
	{
		frameRate = 256.0;
		groundSpeed = 0.0;
	}
}

int StudioModelEntity::GetBodyValueForGroup(int group) const
{
	if (!_editableModel)
	{
		return -1;
	}

	return _editableModel->GetBodyValueForGroup(_bodygroup, group);
}

void StudioModelEntity::SetBodygroup(const int bodygroup, const int value)
{
	if (!_editableModel)
	{
		return;
	}

	if (bodygroup < 0 || bodygroup >= _editableModel->Bodyparts.size())
	{
		return;
	}

	_editableModel->CalculateBodygroup(bodygroup, value, _bodygroup);
}

void StudioModelEntity::SetSkin(const int skin)
{
	if (!_editableModel)
	{
		return;
	}

	if (skin >= 0 && skin < _editableModel->SkinFamilies.size())
	{
		_skin = skin;
	}
}

std::uint8_t StudioModelEntity::GetControllerByIndex(const int controller) const
{
	assert(controller >= 0 && controller < STUDIO_MAX_CONTROLLERS);

	return _controller[controller];
}

float StudioModelEntity::GetControllerValue(const int controller) const
{
	if (!_editableModel)
	{
		return 0;
	}

	if (controller < 0 || controller >= STUDIO_TOTAL_CONTROLLERS)
	{
		return 0;
	}

	return _controllerValues[controller];
}

void StudioModelEntity::SetController(const int controller, float value)
{
	if (!_editableModel)
	{
		return;
	}

	// find first controller that matches the index
	int i;

	for (i = 0; i < _editableModel->BoneControllers.size(); ++i)
	{
		if (_editableModel->BoneControllers[i]->Index == controller)
		{
			break;
		}
	}

	if (i >= _editableModel->BoneControllers.size())
	{
		return;
	}

	_controllerValues[controller] = value;

	const auto& boneController = *_editableModel->BoneControllers[i];

	// wrap 0..360 if it's a rotational controller
	if (boneController.Type & (STUDIO_XR | STUDIO_YR | STUDIO_ZR))
	{
		// ugly hack, invert value if end < start
		if (boneController.End < boneController.Start)
		{
			value = -value;
		}

		// does the controller not wrap?
		if (boneController.Start + 359.0 >= boneController.End)
		{
			if (value > ((static_cast<double>(boneController.Start) + boneController.End) / 2.0) + 180)
			{
				value = value - 360;
			}

			if (value < ((static_cast<double>(boneController.Start) + boneController.End) / 2.0) - 180)
			{
				value = value + 360;
			}
		}
		else
		{
			if (value > 360)
			{
				value = static_cast<float>(value - (int)(value / 360.0) * 360.0);
			}
			else if (value < 0)
			{
				value = static_cast<float>(value + (int)((value / -360.0) + 1) * 360.0);
			}
		}
	}

	int setting = (int)(255 * (value - boneController.Start) /
		(boneController.End - boneController.Start));

	setting = std::clamp(setting, 0, 255);

	_controller[controller] = setting;
}

void StudioModelEntity::SetMouth(float value)
{
	if (!_editableModel)
	{
		return;
	}

	int i;

	// find first controller that matches the mouth
	for (i = 0; i < _editableModel->BoneControllers.size(); ++i)
	{
		if (_editableModel->BoneControllers[i]->Index == STUDIO_MOUTH_CONTROLLER)
		{
			break;
		}
	}

	if (i >= _editableModel->BoneControllers.size())
	{
		return;
	}

	_mouthValue = value;

	const auto& boneController = *_editableModel->BoneControllers[i];

	// wrap 0..360 if it's a rotational controller
	if (boneController.Type & (STUDIO_XR | STUDIO_YR | STUDIO_ZR))
	{
		// ugly hack, invert value if end < start
		if (boneController.End < boneController.Start)
		{
			value = -value;
		}

		// does the controller not wrap?
		if (boneController.Start + 359.0 >= boneController.End)
		{
			if (value > ((static_cast<double>(boneController.Start) + boneController.End) / 2.0) + 180)
			{
				value = value - 360;
			}

			if (value < ((static_cast<double>(boneController.Start) + boneController.End) / 2.0) - 180)
			{
				value = value + 360;
			}
		}
		else
		{
			if (value > 360)
			{
				value = static_cast<float>(value - (int)(value / 360.0) * 360.0);
			}
			else if (value < 0)
			{
				value = static_cast<float>(value + (int)((value / -360.0) + 1) * 360.0);
			}
		}
	}

	int setting = (int)(64 * (value - boneController.Start) / (boneController.End - boneController.Start));

	setting = std::clamp(setting, 0, 64);

	_mouth = setting;
}

std::uint8_t StudioModelEntity::GetBlendingByIndex(const int blender) const
{
	assert(blender >= 0 && blender < STUDIO_MAX_BLENDERS);

	return _blending[blender];
}

float StudioModelEntity::GetBlendingValue(const int blender) const
{
	if (!_editableModel)
	{
		return 0;
	}

	if (blender < 0 || blender >= STUDIO_MAX_BLENDERS)
	{
		return 0;
	}

	if (_sequence < 0 || _sequence >= _editableModel->Sequences.size())
	{
		return 0;
	}

	const auto& sequenceDescriptor = *_editableModel->Sequences[_sequence];

	if (!_blender->AlwaysHasBlender() && sequenceDescriptor.BlendData[blender].Type == 0)
	{
		return 0;
	}

	return _blendingValues[blender];
}

std::optional<std::uint8_t> StudioModelEntity::StandardBlender::CalculateBlend(const studiomdl::Sequence& sequenceDescriptor, int blender, float value) const
{
	if (sequenceDescriptor.BlendData[blender].Type == 0)
	{
		return {};
	}

	if (sequenceDescriptor.BlendData[blender].Type & (STUDIO_XR | STUDIO_YR | STUDIO_ZR))
	{
		// ugly hack, invert value if end < start
		if (sequenceDescriptor.BlendData[blender].End < sequenceDescriptor.BlendData[blender].Start)
		{
			value = -value;
		}

		// does the controller not wrap?
		if (sequenceDescriptor.BlendData[blender].Start + 359.0 >= sequenceDescriptor.BlendData[blender].End)
		{
			if (value > ((static_cast<double>(sequenceDescriptor.BlendData[blender].Start) + sequenceDescriptor.BlendData[blender].End) / 2.0) + 180)
			{
				value = value - 360;
			}

			if (value < ((static_cast<double>(sequenceDescriptor.BlendData[blender].Start) + sequenceDescriptor.BlendData[blender].End) / 2.0) - 180)
			{
				value = value + 360;
			}
		}
	}

	int setting = (int)(255 * (value - sequenceDescriptor.BlendData[blender].Start)
		/ (sequenceDescriptor.BlendData[blender].End - sequenceDescriptor.BlendData[blender].Start));

	setting = std::clamp(setting, 0, 255);

	return setting;
}

std::optional<std::uint8_t> StudioModelEntity::CounterStrikeBlender::CalculateBlend(const studiomdl::Sequence& sequenceDescriptor, int blender, float value) const
{
	switch (blender)
	{
	case SequenceBlendXIndex:
	{
		return static_cast<std::uint8_t>((180.0 + value) / 360.0 * 255.0);
	}

	case SequenceBlendYIndex:
	{
		return static_cast<std::uint8_t>((45 + value) / 90.0 * 255.0);
	}
	}

	//Should never be reached
	assert(!"Invalid blend index");
	return {};
}

void StudioModelEntity::SetBlending(const int blender, float value)
{
	if (!_editableModel)
	{
		return;
	}

	if (blender < 0 || blender >= STUDIO_MAX_BLENDERS)
	{
		return;
	}

	if (_sequence < 0 || _sequence >= _editableModel->Sequences.size())
	{
		return;
	}

	const auto& sequenceDescriptor = *_editableModel->Sequences[_sequence];

	const auto setting = _blender->CalculateBlend(sequenceDescriptor, blender, value);

	if (setting.has_value())
	{
		_blending[blender] = setting.value();
		_blendingValues[blender] = value;
	}
}

void StudioModelEntity::ExtractBbox(glm::vec3& mins, glm::vec3& maxs) const
{
	if (_sequence >= 0 && _sequence < _editableModel->Sequences.size())
	{
		const auto& sequenceDescriptor = *_editableModel->Sequences[_sequence];

		mins = sequenceDescriptor.BBMin;
		maxs = sequenceDescriptor.BBMax;
	}
	else
	{
		mins = glm::vec3{0};
		maxs = glm::vec3{0};
	}
}

void StudioModelEntity::AlignOnGround()
{
	auto model = GetEditableModel();

	//First try finding the idle sequence, since that typically represents a model "at rest"
	//Failing that, use the first sequence
	auto idleFinder = [&]() -> const studiomdl::Sequence*
	{
		if (model->Sequences.empty())
		{
			return nullptr;
		}

		for (const auto& sequence : model->Sequences)
		{
			if (sequence->Label == "idle")
			{
				return sequence.get();
			}
		}

		return model->Sequences[0].get();
	};

	auto sequence = idleFinder();

	SetOrigin({0, 0, sequence ? -sequence->BBMin.z : 0});
}
