#include <cmath>

#include "entity/HLMVStudioModelEntity.hpp"
#include "ui/assets/studiomodel/StudioModelAsset.hpp"
#include "ui/assets/studiomodel/StudioModelUndoCommands.hpp"

namespace ui::assets::studiomodel
{
void ChangeEyePositionCommand::Apply(const glm::vec3& oldValue, const glm::vec3& newValue)
{
	auto model = _asset->GetScene()->GetEntity()->GetEditableModel();
	model->EyePosition = newValue;
}

void ChangeBBoxCommand::Apply(const std::pair<glm::vec3, glm::vec3>& oldValue, const std::pair<glm::vec3, glm::vec3>& newValue)
{
	auto model = _asset->GetScene()->GetEntity()->GetEditableModel();
	model->BoundingMin = newValue.first;
	model->BoundingMax = newValue.second;
}

void ChangeCBoxCommand::Apply(const std::pair<glm::vec3, glm::vec3>& oldValue, const std::pair<glm::vec3, glm::vec3>& newValue)
{
	auto model = _asset->GetScene()->GetEntity()->GetEditableModel();
	model->ClippingMin = newValue.first;
	model->ClippingMax = newValue.second;
}

void BoneRenameCommand::Apply(int index, const QString& oldValue, const QString& newValue)
{
	auto model = _asset->GetScene()->GetEntity()->GetEditableModel();
	auto& bone = *model->Bones[index];

	bone.Name = newValue.toStdString();
}

void ChangeBoneParentCommand::Apply(int index, const int& oldValue, const int& newValue)
{
	auto model = _asset->GetScene()->GetEntity()->GetEditableModel();
	auto& bone = *model->Bones[index];

	bone.Parent = newValue != -1 ? model->Bones[newValue].get() : nullptr;
}

void ChangeBoneFlagsCommand::Apply(int index, const int& oldValue, const int& newValue)
{
	auto model = _asset->GetScene()->GetEntity()->GetEditableModel();
	auto& bone = *model->Bones[index];

	bone.Flags = newValue;
}

void ChangeBonePropertyCommand::Apply(int index, const ChangeBoneProperties& oldValue, const ChangeBoneProperties& newValue)
{
	auto model = _asset->GetScene()->GetEntity()->GetEditableModel();
	auto& bone = *model->Bones[index];

	for (std::size_t j = 0; j < newValue.Values.size(); ++j)
	{
		for (int i = 0; i < newValue.Values[j].length(); ++i)
		{
			bone.Axes[(j * newValue.Values[j].length()) + i].Value = newValue.Values[j][i];
			bone.Axes[(j * newValue.Scales[j].length()) + i].Scale = newValue.Scales[j][i];
		}
	}
}

void ChangeBoneControllerFromBoneCommand::Apply(int index, const int& oldValue, const int& newValue)
{
	auto model = _asset->GetScene()->GetEntity()->GetEditableModel();
	
	auto oldController = oldValue != -1 ? model->BoneControllers[oldValue].get() : nullptr;
	auto newController = newValue != -1 ? model->BoneControllers[newValue].get() : nullptr;

	//Detach old controller, if any
	if (oldController)
	{
		oldController->Type = 0;
	}

	//Set up new controller to attach to bone
	if (newController)
	{
		//Find any other bones (including target bone) that have the new controller tied to it
		//and remove it (moves controller from bone axis to bone axis)
		for ( auto& bone : model->Bones)
		{
			for (auto& axis : bone->Axes)
			{
				if (axis.Controller == newController)
				{
					axis.Controller = nullptr;
				}
			}
		}

		newController->Type = 1 << _boneControllerAxis;
	}

	{
		auto& bone = *model->Bones[index];
		bone.Axes[_boneControllerAxis].Controller = newController;
	}
}

void ChangeAttachmentNameCommand::Apply(int index, const QString& oldValue, const QString& newValue)
{
	auto model = _asset->GetScene()->GetEntity()->GetEditableModel();
	auto& attachment = *model->Attachments[index];

	attachment.Name = newValue.toStdString();
}

void ChangeAttachmentTypeCommand::Apply(int index, const int& oldValue, const int& newValue)
{
	auto model = _asset->GetScene()->GetEntity()->GetEditableModel();
	auto& attachment = *model->Attachments[index];

	attachment.Type = newValue;
}

void ChangeAttachmentBoneCommand::Apply(int index, const int& oldValue, const int& newValue)
{
	auto model = _asset->GetScene()->GetEntity()->GetEditableModel();
	auto& attachment = *model->Attachments[index];

	attachment.Bone = newValue != -1 ? model->Bones[index].get() : nullptr;
}

void ChangeAttachmentOriginCommand::Apply(int index, const glm::vec3& oldValue, const glm::vec3& newValue)
{
	auto model = _asset->GetScene()->GetEntity()->GetEditableModel();
	auto& attachment = *model->Attachments[index];

	for (int i = 0; i < newValue.length(); ++i)
	{
		attachment.Origin[i] = newValue[i];
	}
}

void ChangeBoneControllerRangeCommand::Apply(int index, const ChangeBoneControllerRange& oldValue, const ChangeBoneControllerRange& newValue)
{
	auto model = _asset->GetScene()->GetEntity()->GetEditableModel();
	auto& controller = *model->BoneControllers[index];

	controller.Start = newValue.Start;
	controller.End = newValue.End;
}

void ChangeBoneControllerRestCommand::Apply(int index, const int& oldValue, const int& newValue)
{
	auto model = _asset->GetScene()->GetEntity()->GetEditableModel();
	auto& controller = *model->BoneControllers[index];

	controller.Rest = newValue;
}

void ChangeBoneControllerIndexCommand::Apply(int index, const int& oldValue, const int& newValue)
{
	auto model = _asset->GetScene()->GetEntity()->GetEditableModel();
	auto& controller = *model->BoneControllers[index];

	controller.Index = newValue;
}

void ChangeBoneControllerFromControllerCommand::Apply(int index, const ChangeBoneControllerData& oldValue, const ChangeBoneControllerData& newValue)
{
	auto model = _asset->GetScene()->GetEntity()->GetEditableModel();

	auto& controller = *model->BoneControllers[index];

	auto oldBone = oldValue.first != -1 ? model->Bones[oldValue.first].get() : nullptr;
	auto newBone = newValue.first != -1 ? model->Bones[newValue.first].get() : nullptr;

	//Detach from old bone, if any
	if (oldBone)
	{
		oldBone->Axes[oldValue.second].Controller = nullptr;
	}

	//Attach to new bone
	if (newBone)
	{
		newBone->Axes[newValue.second].Controller = &controller;
		controller.Type = 1 << newValue.second;
	}
	else
	{
		controller.Type = 0;
	}
}

void ChangeModelFlagsCommand::Apply(const int& oldValue, const int& newValue)
{
	auto model = _asset->GetScene()->GetEntity()->GetEditableModel();

	model->Flags = newValue;
}

void ChangeModelOriginCommand::Apply(const ChangeModelOriginData& oldValue, const ChangeModelOriginData& newValue)
{
	auto model = _asset->GetScene()->GetEntity()->GetEditableModel();

	for (const auto& data : newValue.BoneData)
	{
		auto& bone = *model->Bones[data.BoneIndex];

		for (int i = 0; i < data.BonePosition.length(); ++i)
		{
			bone.Axes[i].Value = data.BonePosition[i];
		}
	}
}

void ChangeModelMeshesScaleCommand::Apply(const studiomdl::ScaleMeshesData& oldValue, const studiomdl::ScaleMeshesData& newValue)
{
	ApplyScaleMeshesData(*_asset->GetScene()->GetEntity()->GetEditableModel(), newValue);
}

void ChangeModelBonesScaleCommand::Apply(const std::vector<studiomdl::ScaleBonesBoneData>& oldValue, const std::vector<studiomdl::ScaleBonesBoneData>& newValue)
{
	ApplyScaleBonesData(*_asset->GetScene()->GetEntity()->GetEditableModel(), newValue);
}

void ChangeHitboxBoneCommand::Apply(int index, const int& oldValue, const int& newValue)
{
	auto model = _asset->GetScene()->GetEntity()->GetEditableModel();
	auto& hitbox = *model->Hitboxes[index];

	hitbox.Bone = model->Bones[newValue].get();
}

void ChangeHitboxHitgroupCommand::Apply(int index, const int& oldValue, const int& newValue)
{
	auto model = _asset->GetScene()->GetEntity()->GetEditableModel();
	auto& hitbox = *model->Hitboxes[index];

	hitbox.Group = newValue;
}

void ChangeHitboxBoundsCommand::Apply(int index, const std::pair<glm::vec3, glm::vec3>& oldValue, const std::pair<glm::vec3, glm::vec3>& newValue)
{
	auto model = _asset->GetScene()->GetEntity()->GetEditableModel();
	auto& hitbox = *model->Hitboxes[index];

	hitbox.Min = newValue.first;
	hitbox.Max = newValue.second;
}

void ChangeTextureNameCommand::Apply(int index, const QString& oldValue, const QString& newValue)
{
	auto model = _asset->GetScene()->GetEntity()->GetEditableModel();
	auto& texture = *model->Textures[index];

	texture.Name = newValue.toStdString();
}

void ChangeTextureFlagsCommand::Apply(int index, const int& oldValue, const int& newValue)
{
	auto model = _asset->GetScene()->GetEntity()->GetEditableModel();
	auto& texture = *model->Textures[index];

	texture.Flags = newValue;
}

void ImportTextureCommand::Apply(int index, const ImportTextureData& oldValue, const ImportTextureData& newValue)
{
	auto model = _asset->GetScene()->GetEntity()->GetEditableModel();
	auto& texture = *model->Textures[index];

	texture.Width = newValue.Width;
	texture.Height = newValue.Height;

	texture.Pixels.resize(newValue.Width * newValue.Height);

	//Copy over the new image data to the texture
	memcpy(texture.Pixels.data(), newValue.Pixels.get(), newValue.Width * newValue.Height);
	texture.Palette = newValue.Palette;

	model->ReplaceTexture(*_asset->GetTextureLoader(), &texture, newValue.Pixels.get(), newValue.Palette);

	studiomdl::ApplyScaledSTCoordinatesData(*model, index, newValue.ScaledSTCoordinates);
}

void ChangeEventCommand::Apply(int index, const studiomdl::SequenceEvent& oldValue, const studiomdl::SequenceEvent& newValue)
{
	auto model = _asset->GetScene()->GetEntity()->GetEditableModel();
	auto& sequence = *model->Sequences[index];

	*sequence.Events[_eventIndex] = newValue;

	//Sort again if needed
	if (oldValue.Frame != newValue.Frame)
	{
		studiomdl::SortEventsList(sequence.SortedEvents);
	}
}

void AddRemoveEventCommand::Add(int index, const studiomdl::SequenceEvent& value)
{
	auto model = _asset->GetScene()->GetEntity()->GetEditableModel();
	auto& sequence = *model->Sequences[index];

	auto event = sequence.Events.insert(sequence.Events.begin() + _eventIndex, std::make_unique<studiomdl::SequenceEvent>(value))->get();
	sequence.SortedEvents.push_back(event);

	studiomdl::SortEventsList(sequence.SortedEvents);
}

void AddRemoveEventCommand::Remove(int index, const studiomdl::SequenceEvent& value)
{
	auto model = _asset->GetScene()->GetEntity()->GetEditableModel();
	auto& sequence = *model->Sequences[index];

	sequence.SortedEvents.erase(
		std::remove(sequence.SortedEvents.begin(), sequence.SortedEvents.end(), sequence.Events[_eventIndex].get()), sequence.SortedEvents.end());
	sequence.Events.erase(sequence.Events.begin() + _eventIndex);
}

void ChangeModelNameCommand::Apply(int index, const QString& oldValue, const QString& newValue)
{
	auto model = _asset->GetScene()->GetEntity()->GetEditableModel();
	auto& bodyPart = *model->Bodyparts[index];
	auto& subModel = bodyPart.Models[_modelIndex];

	subModel.Name = newValue.toStdString();
}
}
