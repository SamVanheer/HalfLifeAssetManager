#include <algorithm>

#include <glm/geometric.hpp>

#include "core/shared/Logging.hpp"
#include "core/shared/WorldTime.hpp"

#include "engine/shared/studiomodel/StudioModel.hpp"

#include "game/entity/StudioModelEntity.hpp"

bool StudioModelEntity::Spawn()
{
	SetSequence(0);
	SetController(0, 0.0f);
	SetController(1, 0.0f);
	SetController(2, 0.0f);
	SetController(3, 0.0f);
	SetMouth(0.0f);

	const studiohdr_t* header = _model->GetStudioHeader();

	for (int n = 0; n < header->numbodyparts; ++n)
	{
		SetBodygroup(n, 0);
	}

	SetSkin(0);

	return true;
}

void StudioModelEntity::Draw(renderer::DrawFlags flags)
{
	studiomdl::ModelRenderInfo renderInfo = GetRenderInfo();

	GetContext()->StudioModelRenderer->DrawModel(&renderInfo, flags);
}

studiomdl::ModelRenderInfo StudioModelEntity::GetRenderInfo() const
{
	studiomdl::ModelRenderInfo renderInfo{};

	renderInfo.Origin = GetOrigin();
	renderInfo.Angles = GetAngles();
	renderInfo.Scale = GetScale();

	renderInfo.Model = GetModel();

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
	if (!_model)
	{
		return 0;
	}

	const studiohdr_t* header = _model->GetStudioHeader();

	const mstudioseqdesc_t* sequenceDescriptor = header->GetSequence(_sequence);

	if (deltaTime == 0)
	{
		deltaTime = (GetContext()->Time->GetTime() - _animTime);

		if (deltaTime <= 0.001)
		{
			_animTime = GetContext()->Time->GetTime();
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
		shouldLoop = (sequenceDescriptor->flags & STUDIO_LOOPING) != 0;
		break;
	}

	const float increment = deltaTime * sequenceDescriptor->fps * _frameRate;

	if (_frame < (sequenceDescriptor->numframes - 1) || shouldLoop)
	{
		_frame += increment;
	}

	if (sequenceDescriptor->numframes <= 1)
	{
		_frame = 0;
	}
	else
	{
		if (shouldLoop)
		{
			// wrap
			_frame -= (int)(_frame / (sequenceDescriptor->numframes - 1)) * (sequenceDescriptor->numframes - 1);
		}
		else if (_frame >= (sequenceDescriptor->numframes - 1))
		{
			//Clamp frame to the last valid frame index
			_frame = static_cast<float>(sequenceDescriptor->numframes - 1);
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

int StudioModelEntity::GetAnimationEvent(AnimEvent& event, float start, float end, int index, const bool allowClientEvents)
{
	if (!_model)
	{
		return 0;
	}

	const studiohdr_t* header = _model->GetStudioHeader();

	if (_sequence >= header->numseq)
	{
		return 0;
	}

	int events = 0;

	const mstudioseqdesc_t* sequenceDescriptor = header->GetSequence(_sequence);
	const mstudioevent_t* pevent = (const mstudioevent_t*)((const byte*)header + sequenceDescriptor->eventindex);

	if (sequenceDescriptor->numevents == 0 || index > sequenceDescriptor->numevents)
	{
		return 0;
	}

	if (sequenceDescriptor->numframes <= 1)
	{
		start = 0;
		end = 1.0;
	}

	for (; index < sequenceDescriptor->numevents; index++)
	{
		//TODO: maybe leave it up to the listener to filter these out?
		if (!allowClientEvents)
		{
			// Don't send client-side events to the server AI
			if (pevent[index].event >= EVENT_CLIENT)
			{
				continue;
			}
		}

		if ((pevent[index].frame >= start && pevent[index].frame < end) ||
			((sequenceDescriptor->flags & STUDIO_LOOPING)
				&& end >= sequenceDescriptor->numframes - 1
				&& pevent[index].frame < end - sequenceDescriptor->numframes + 1))
		{
			event.id = pevent[index].event;
			event.options = pevent[index].options;
			return index + 1;
		}
	}

	return 0;
}

void StudioModelEntity::DispatchAnimEvents(const bool allowClientEvents)
{
	if (!_model)
	{
		Message("Gibbed monster is thinking!\n");
		return;
	}

	//This is based on Source's DispatchAnimEvents. It fixes the bug where events don't get triggered, and get triggered multiple times due to the workaround.
	//Plays from previous frame to current. This differs from GoldSource in that GoldSource plays from current to predicted future frame.
	//This is more accurate, since it's based on actual frame data, rather than predicted frames, but results in events firing later than before.
	//The difference is ~0.1 seconds when running at 60 FPS. On dedicated servers the difference will be smaller if the tick rate is higher.
	float start = _lastEventCheck;
	float end = _frame;
	_lastEventCheck = _frame;

	AnimEvent event;

	int index = 0;

	while ((index = GetAnimationEvent(event, start, end, index, allowClientEvents)) != 0)
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

	if (!_model)
	{
		return;
	}

	const mstudioseqdesc_t* sequenceDescriptor = _model->GetStudioHeader()->GetSequence(_sequence);

	_frame = frame;

	if (sequenceDescriptor->numframes <= 1)
	{
		_frame = 0;
	}
	else
	{
		// wrap
		_frame -= (int)(_frame / (sequenceDescriptor->numframes - 1)) * (sequenceDescriptor->numframes - 1);
	}

	_animTime = GetContext()->Time->GetTime();
}

void StudioModelEntity::SetModel(studiomdl::StudioModel* model)
{
	//TODO: release old model.
	_model = model;

	//TODO: reinit entity settings
}

int StudioModelEntity::GetNumFrames() const
{
	const mstudioseqdesc_t* const sequenceDescriptor = _model->GetStudioHeader()->GetSequence(_sequence);

	return sequenceDescriptor->numframes;
}

void StudioModelEntity::SetSequence(const int sequence)
{
	if (sequence < 0 || sequence >= _model->GetStudioHeader()->numseq)
	{
		return;
	}

	_sequence = sequence;
	_frame = 0;
	_lastEventCheck = 0;
}

void StudioModelEntity::GetSequenceInfo(float& frameRate, float& groundSpeed) const
{
	const mstudioseqdesc_t* sequenceDescriptor = _model->GetStudioHeader()->GetSequence(_sequence);

	if (sequenceDescriptor->numframes > 1)
	{
		frameRate = sequenceDescriptor->fps;
		groundSpeed = static_cast<float>(glm::length(sequenceDescriptor->linearmovement));
		groundSpeed = groundSpeed * sequenceDescriptor->fps / (sequenceDescriptor->numframes - 1);
	}
	else
	{
		frameRate = 256.0;
		groundSpeed = 0.0;
	}
}

int StudioModelEntity::GetBodyValueForGroup(int group) const
{
	if (!_model)
	{
		return -1;
	}

	return _model->GetBodyValueForGroup(_bodygroup, group);
}

void StudioModelEntity::SetBodygroup(const int bodygroup, const int value)
{
	if (!_model)
	{
		return;
	}

	if (bodygroup < 0 || bodygroup >= _model->GetStudioHeader()->numbodyparts)
	{
		return;
	}

	_model->CalculateBodygroup(bodygroup, value, _bodygroup);
}

void StudioModelEntity::SetSkin(const int skin)
{
	if (!_model)
	{
		return;
	}

	if (skin >= 0 && skin < _model->GetTextureHeader()->numskinfamilies)
	{
		_skin = skin;
	}
}

byte StudioModelEntity::GetControllerByIndex(const int controller) const
{
	assert(controller >= 0 && controller < STUDIO_MAX_CONTROLLERS);

	return _controller[controller];
}

float StudioModelEntity::GetControllerValue(const int controller) const
{
	if (!_model)
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
	if (!_model)
	{
		return;
	}

	const studiohdr_t* header = _model->GetStudioHeader();

	const mstudiobonecontroller_t* boneController = header->GetBoneControllers();

	// find first controller that matches the index
	int i;

	for (i = 0; i < header->numbonecontrollers; ++i, ++boneController)
	{
		if (boneController->index == controller)
		{
			break;
		}
	}

	if (i >= header->numbonecontrollers)
	{
		return;
	}

	_controllerValues[controller] = value;

	// wrap 0..360 if it's a rotational controller
	if (boneController->type & (STUDIO_XR | STUDIO_YR | STUDIO_ZR))
	{
		// ugly hack, invert value if end < start
		if (boneController->end < boneController->start)
		{
			value = -value;
		}

		// does the controller not wrap?
		if (boneController->start + 359.0 >= boneController->end)
		{
			if (value > ((static_cast<double>(boneController->start) + boneController->end) / 2.0) + 180)
			{
				value = value - 360;
			}

			if (value < ((static_cast<double>(boneController->start) + boneController->end) / 2.0) - 180)
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

	int setting = (int)(255 * (value - boneController->start) /
		(boneController->end - boneController->start));

	setting = std::clamp(setting, 0, 255);

	_controller[controller] = setting;
}

void StudioModelEntity::SetMouth(float value)
{
	if (!_model)
	{
		return;
	}

	const studiohdr_t* header = _model->GetStudioHeader();

	const mstudiobonecontroller_t* boneController = header->GetBoneControllers();

	// find first controller that matches the mouth
	for (int i = 0; i < header->numbonecontrollers; ++i, ++boneController)
	{
		if (boneController->index == STUDIO_MOUTH_CONTROLLER)
		{
			break;
		}
	}

	// wrap 0..360 if it's a rotational controller
	if (boneController->type & (STUDIO_XR | STUDIO_YR | STUDIO_ZR))
	{
		// ugly hack, invert value if end < start
		if (boneController->end < boneController->start)
		{
			value = -value;
		}

		// does the controller not wrap?
		if (boneController->start + 359.0 >= boneController->end)
		{
			if (value > ((static_cast<double>(boneController->start) + boneController->end) / 2.0) + 180)
			{
				value = value - 360;
			}

			if (value < ((static_cast<double>(boneController->start) + boneController->end) / 2.0) - 180)
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

	int setting = (int)(64 * (value - boneController->start) / (boneController->end - boneController->start));

	setting = std::clamp(setting, 0, 64);

	_mouth = setting;
}

byte StudioModelEntity::GetBlendingByIndex(const int blender) const
{
	assert(blender >= 0 && blender < STUDIO_MAX_BLENDERS);

	return _blending[blender];
}

float StudioModelEntity::GetBlendingValue(const int blender) const
{
	if (!_model)
	{
		return 0;
	}

	if (blender < 0 || blender >= STUDIO_MAX_BLENDERS)
	{
		return 0;
	}

	const studiohdr_t* header = _model->GetStudioHeader();

	const mstudioseqdesc_t* sequenceDescriptor = header->GetSequence(_sequence);

	if (sequenceDescriptor->blendtype[blender] == 0)
	{
		return 0;
	}

	return static_cast<float>(_blending[blender] * (1.0 / 255.0)
		* (static_cast<double>(sequenceDescriptor->blendend[blender]) - sequenceDescriptor->blendstart[blender]) + sequenceDescriptor->blendstart[blender]);
}

void StudioModelEntity::SetBlending(const int blender, float value)
{
	if (!_model)
	{
		return;
	}

	if (blender < 0 || blender >= STUDIO_MAX_BLENDERS)
	{
		return;
	}

	const studiohdr_t* header = _model->GetStudioHeader();

	const mstudioseqdesc_t* sequenceDescriptor = header->GetSequence(_sequence);

	if (sequenceDescriptor->blendtype[blender] == 0)
	{
		return;
	}

	if (sequenceDescriptor->blendtype[blender] & (STUDIO_XR | STUDIO_YR | STUDIO_ZR))
	{
		// ugly hack, invert value if end < start
		if (sequenceDescriptor->blendend[blender] < sequenceDescriptor->blendstart[blender])
		{
			value = -value;
		}

		// does the controller not wrap?
		if (sequenceDescriptor->blendstart[blender] + 359.0 >= sequenceDescriptor->blendend[blender])
		{
			if (value > ((static_cast<double>(sequenceDescriptor->blendstart[blender]) + sequenceDescriptor->blendend[blender]) / 2.0) + 180)
			{
				value = value - 360;
			}

			if (value < ((static_cast<double>(sequenceDescriptor->blendstart[blender]) + sequenceDescriptor->blendend[blender]) / 2.0) - 180)
			{
				value = value + 360;
			}
		}
	}

	int setting = (int)(255 * (value - sequenceDescriptor->blendstart[blender])
		/ (sequenceDescriptor->blendend[blender] - sequenceDescriptor->blendstart[blender]));

	setting = std::clamp(setting, 0, 255);

	_blending[blender] = setting;
}

void StudioModelEntity::SetCounterStrikeBlending(const int blender, float value)
{
	switch (blender)
	{
	case SequenceBlendXIndex:
	{
		_blending[blender] = static_cast<byte>((180.0 + value) / 360.0 * 255.0);
		break;
	}

	case SequenceBlendYIndex:
	{
		_blending[blender] = static_cast<byte>((45 + value) / 90.0 * 255.0);
		break;
	}
	}
}

void StudioModelEntity::ExtractBbox(glm::vec3& mins, glm::vec3& maxs) const
{
	const mstudioseqdesc_t* sequenceDescriptor = _model->GetStudioHeader()->GetSequence(_sequence);

	mins = sequenceDescriptor->bbmin;
	maxs = sequenceDescriptor->bbmax;
}

mstudiomodel_t* StudioModelEntity::GetModelByBodyPart(const int bodyPart) const
{
	return _model->GetModelByBodyPart(_bodygroup, bodyPart);
}

std::vector<const mstudiomesh_t*> StudioModelEntity::ComputeMeshList(const int texture) const
{
	if (!_model)
	{
		return {};
	}

	std::vector<const mstudiomesh_t*> meshes;

	const auto header = _model->GetStudioHeader();

	const auto textureHeader = _model->GetTextureHeader();

	const short* const skinRef = textureHeader->GetSkins();

	int iBodygroup = 0;

	for (int iBodyPart = 0; iBodyPart < header->numbodyparts; ++iBodyPart)
	{
		mstudiobodyparts_t* pbodypart = header->GetBodypart(iBodyPart);

		for (int iModel = 0; iModel < pbodypart->nummodels; ++iModel)
		{
			_model->CalculateBodygroup(iBodyPart, iModel, iBodygroup);

			mstudiomodel_t* pModel = _model->GetModelByBodyPart(iBodygroup, iBodyPart);

			for (int iMesh = 0; iMesh < pModel->nummesh; ++iMesh)
			{
				const mstudiomesh_t* mesh = ((const mstudiomesh_t*)((const byte*)_model->GetStudioHeader() + pModel->meshindex)) + iMesh;

				//Check each skin family to detect textures used only by alternate skins (e.g. scientist hands)
				for (int skinFamily = 0; skinFamily < textureHeader->numskinfamilies; ++skinFamily)
				{
					if (skinRef[(skinFamily * textureHeader->numskinref) + mesh->skinref] == texture)
					{
						meshes.push_back(mesh);
						break;
					}
				}
			}
		}
	}

	return meshes;
}
