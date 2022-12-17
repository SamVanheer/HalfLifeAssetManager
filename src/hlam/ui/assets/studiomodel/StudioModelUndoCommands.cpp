#include <cmath>

#include <QAbstractItemModel>

#include "entity/HLMVStudioModelEntity.hpp"
#include "graphics/IGraphicsContext.hpp"
#include "graphics/Scene.hpp"
#include "ui/assets/studiomodel/StudioModelAsset.hpp"
#include "ui/assets/studiomodel/StudioModelData.hpp"
#include "ui/assets/studiomodel/StudioModelUndoCommands.hpp"

namespace studiomodel
{
void BaseModelUndoCommand::EmitDataChanged(QAbstractItemModel* model, int index)
{
	const auto modelIndex = model->index(index, 0);
	emit model->dataChanged(modelIndex, modelIndex);
}

void ChangeEyePositionCommand::Apply(const glm::vec3& oldValue, const glm::vec3& newValue)
{
	_asset->GetEditableStudioModel()->EyePosition = newValue;
	emit _asset->GetModelData()->EyePositionChanged();
}

void ChangeBBoxCommand::Apply(const std::pair<glm::vec3, glm::vec3>& oldValue, const std::pair<glm::vec3, glm::vec3>& newValue)
{
	auto model = _asset->GetEditableStudioModel();
	model->BoundingMin = newValue.first;
	model->BoundingMax = newValue.second;
	emit _asset->GetModelData()->ModelBBoxChanged();
}

void ChangeCBoxCommand::Apply(const std::pair<glm::vec3, glm::vec3>& oldValue, const std::pair<glm::vec3, glm::vec3>& newValue)
{
	auto model = _asset->GetEditableStudioModel();
	model->ClippingMin = newValue.first;
	model->ClippingMax = newValue.second;
	emit _asset->GetModelData()->ModelCBoxChanged();
}

void BoneRenameCommand::Apply(int index, const QString& oldValue, const QString& newValue)
{
	_asset->GetEditableStudioModel()->Bones[index]->Name = newValue.toStdString();
	EmitDataChanged(_asset->GetModelData()->Bones, index);
	EmitDataChanged(_asset->GetModelData()->BonesWithNone, index + 1);
}

void ChangeBoneParentCommand::Apply(int index, const int& oldValue, const int& newValue)
{
	auto model = _asset->GetEditableStudioModel();
	model->Bones[index]->Parent = newValue != -1 ? model->Bones[newValue].get() : nullptr;
	emit _asset->GetModelData()->BoneDataChanged(index);
}

void ChangeBoneFlagsCommand::Apply(int index, const int& oldValue, const int& newValue)
{
	_asset->GetEditableStudioModel()->Bones[index]->Flags = newValue;
	emit _asset->GetModelData()->BoneDataChanged(index);
}

void ChangeBonePropertyCommand::Apply(int index, const ChangeBoneProperties& oldValue, const ChangeBoneProperties& newValue)
{
	auto& bone = *_asset->GetEditableStudioModel()->Bones[index];

	for (std::size_t j = 0; j < newValue.Values.size(); ++j)
	{
		for (int i = 0; i < newValue.Values[j].length(); ++i)
		{
			bone.Axes[(j * newValue.Values[j].length()) + i].Value = newValue.Values[j][i];
			bone.Axes[(j * newValue.Scales[j].length()) + i].Scale = newValue.Scales[j][i];
		}
	}

	emit _asset->GetModelData()->BoneDataChanged(index);
}

void ChangeBoneControllerFromBoneCommand::Apply(int index, const int& oldValue, const int& newValue)
{
	auto model = _asset->GetEditableStudioModel();
	
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

	emit _asset->GetModelData()->BoneControllerDataChanged(oldValue);
	emit _asset->GetModelData()->BoneControllerDataChanged(newValue);
}

void ChangeAttachmentNameCommand::Apply(int index, const QString& oldValue, const QString& newValue)
{
	_asset->GetEditableStudioModel()->Attachments[index]->Name = newValue.toStdString();
	emit _asset->GetModelData()->AttachmentDataChanged(index);
	EmitDataChanged(_asset->GetModelData()->Attachments, index);
}

void ChangeAttachmentTypeCommand::Apply(int index, const int& oldValue, const int& newValue)
{
	_asset->GetEditableStudioModel()->Attachments[index]->Type = newValue;
	emit _asset->GetModelData()->AttachmentDataChanged(index);
}

void ChangeAttachmentBoneCommand::Apply(int index, const int& oldValue, const int& newValue)
{
	auto model = _asset->GetEditableStudioModel();
	model->Attachments[index]->Bone = newValue != -1 ? model->Bones[newValue].get() : nullptr;
	emit _asset->GetModelData()->AttachmentDataChanged(index);
}

void ChangeAttachmentOriginCommand::Apply(int index, const glm::vec3& oldValue, const glm::vec3& newValue)
{
	_asset->GetEditableStudioModel()->Attachments[index]->Origin = newValue;
	emit _asset->GetModelData()->AttachmentDataChanged(index);
}

void ChangeBoneControllerRangeCommand::Apply(int index, const ChangeBoneControllerRange& oldValue, const ChangeBoneControllerRange& newValue)
{
	auto& controller = *_asset->GetEditableStudioModel()->BoneControllers[index];

	controller.Start = newValue.Start;
	controller.End = newValue.End;

	emit _asset->GetModelData()->BoneControllerDataChanged(index);
}

void ChangeBoneControllerRestCommand::Apply(int index, const int& oldValue, const int& newValue)
{
	_asset->GetEditableStudioModel()->BoneControllers[index]->Rest = newValue;
	emit _asset->GetModelData()->BoneControllerDataChanged(index);
}

void ChangeBoneControllerIndexCommand::Apply(int index, const int& oldValue, const int& newValue)
{
	_asset->GetEditableStudioModel()->BoneControllers[index]->Index = newValue;
	emit _asset->GetModelData()->BoneControllerDataChanged(index);
}

void ChangeBoneControllerFromControllerCommand::Apply(int index, const ChangeBoneControllerData& oldValue, const ChangeBoneControllerData& newValue)
{
	auto model = _asset->GetEditableStudioModel();

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

	emit _asset->GetModelData()->BoneControllerDataChanged(index);
}

void ChangeModelFlagsCommand::Apply(const int& oldValue, const int& newValue)
{
	_asset->GetEditableStudioModel()->Flags = newValue;
	emit _asset->GetModelData()->ModelFlagsChanged();
}

void ChangeModelOriginCommand::Apply(const studiomdl::MoveData& oldValue, const studiomdl::MoveData& newValue)
{
	ApplyMoveData(*_asset->GetEditableStudioModel(), newValue);
	emit _asset->GetModelData()->ModelOriginChanged();
}

void ChangeModelScaleCommand::Apply(const studiomdl::ScaleData& oldValue, const studiomdl::ScaleData& newValue)
{
	ApplyScaleData(*_asset->GetEditableStudioModel(), newValue);
	emit _asset->GetModelData()->ModelScaleChanged();
}

void ChangeModelRotationCommand::Apply(const studiomdl::RotateData& oldValue, const studiomdl::RotateData& newValue)
{
	ApplyRotateData(*_asset->GetEditableStudioModel(), newValue);
	emit _asset->GetModelData()->ModelRotationChanged();
}

void ChangeHitboxBoneCommand::Apply(int index, const int& oldValue, const int& newValue)
{
	auto model = _asset->GetEditableStudioModel();
	model->Hitboxes[index]->Bone = model->Bones[newValue].get();
	emit _asset->GetModelData()->HitboxDataChanged(index);
}

void ChangeHitboxHitgroupCommand::Apply(int index, const int& oldValue, const int& newValue)
{
	_asset->GetEditableStudioModel()->Hitboxes[index]->Group = newValue;
	emit _asset->GetModelData()->HitboxDataChanged(index);
}

void ChangeHitboxBoundsCommand::Apply(int index, const std::pair<glm::vec3, glm::vec3>& oldValue, const std::pair<glm::vec3, glm::vec3>& newValue)
{
	auto& hitbox = *_asset->GetEditableStudioModel()->Hitboxes[index];
	hitbox.Min = newValue.first;
	hitbox.Max = newValue.second;
	emit _asset->GetModelData()->HitboxDataChanged(index);
}

void ChangeTextureNameCommand::Apply(int index, const QString& oldValue, const QString& newValue)
{
	_asset->GetEditableStudioModel()->Textures[index]->Name = newValue.toStdString();
	emit _asset->GetModelData()->TextureNameChanged(index);
	EmitDataChanged(_asset->GetModelData()->Textures, index);
}

void ChangeTextureFlagsCommand::Apply(int index, const int& oldValue, const int& newValue)
{
	auto model = _asset->GetEditableStudioModel();
	model->Textures[index]->Flags = newValue;

	auto graphicsContext = _asset->GetGraphicsContext();

	graphicsContext->Begin();
	model->UpdateTexture(*_asset->GetTextureLoader(), index);
	graphicsContext->End();

	emit _asset->GetModelData()->TextureFlagsChanged(index);
}

void ImportTextureCommand::Apply(int index, const ImportTextureData& oldValue, const ImportTextureData& newValue)
{
	auto model = _asset->GetEditableStudioModel();
	model->Textures[index]->Data = newValue.Data;

	auto graphicsContext = _asset->GetGraphicsContext();

	graphicsContext->Begin();
	model->UpdateTexture(*_asset->GetTextureLoader(), index);
	graphicsContext->End();

	studiomdl::ApplyScaledSTCoordinatesData(*model, index, newValue.ScaledSTCoordinates);
}

void ChangeEventCommand::Apply(int index, const studiomdl::SequenceEvent& oldValue, const studiomdl::SequenceEvent& newValue)
{
	auto& sequence = *_asset->GetEditableStudioModel()->Sequences[index];

	*sequence.Events[_eventIndex] = newValue;

	//Sort again if needed
	if (oldValue.Frame != newValue.Frame)
	{
		studiomdl::SortEventsList(sequence.SortedEvents);
	}

	emit _asset->GetModelData()->EventChanged(index, _eventIndex);
}

void AddRemoveEventCommand::Add(int index, const studiomdl::SequenceEvent& value)
{
	auto& sequence = *_asset->GetEditableStudioModel()->Sequences[index];

	auto event = sequence.Events.insert(sequence.Events.begin() + _eventIndex, std::make_unique<studiomdl::SequenceEvent>(value))->get();
	sequence.SortedEvents.push_back(event);

	studiomdl::SortEventsList(sequence.SortedEvents);
	emit _asset->GetModelData()->EventAdded(index, _eventIndex);
}

void AddRemoveEventCommand::Remove(int index, const studiomdl::SequenceEvent& value)
{
	auto& sequence = *_asset->GetEditableStudioModel()->Sequences[index];

	sequence.SortedEvents.erase(
		std::remove(sequence.SortedEvents.begin(), sequence.SortedEvents.end(), sequence.Events[_eventIndex].get()), sequence.SortedEvents.end());
	sequence.Events.erase(sequence.Events.begin() + _eventIndex);
	emit _asset->GetModelData()->EventRemoved(index, _eventIndex);
}

void ChangeModelNameCommand::Apply(int index, const QString& oldValue, const QString& newValue)
{
	_asset->GetEditableStudioModel()->Bodyparts[index]->Models[_modelIndex].Name = newValue.toStdString();
	emit _asset->GetModelData()->SubModelNameChanged(index, _modelIndex);
}

void FlipNormalsCommand::Apply(int index, const std::vector<glm::vec3>& oldValue, const std::vector<glm::vec3>& newValue)
{
	auto model = _asset->GetEditableStudioModel();

	std::size_t normalIndex = 0;
	
	for (auto& bodypart : model->Bodyparts)
	{
		for (auto& model : bodypart->Models)
		{
			for (auto& normal : model.Normals)
			{
				normal.Vertex = newValue[normalIndex++];
			}
		}
	}
}
}
