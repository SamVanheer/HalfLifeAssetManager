#include <cmath>

#include <QAbstractItemModel>

#include "entity/HLMVStudioModelEntity.hpp"
#include "graphics/IGraphicsContext.hpp"
#include "graphics/Scene.hpp"
#include "plugins/halflife/studiomodel/StudioModelAsset.hpp"
#include "plugins/halflife/studiomodel/ui/StudioModelData.hpp"
#include "plugins/halflife/studiomodel/ui/StudioModelUndoCommands.hpp"

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

void ChangeBonePropsCommand::Apply(int index, const BoneProps& oldValue, const BoneProps& newValue)
{
	auto model = _asset->GetEditableStudioModel();
	auto& bone = *model->Bones[index];

	bone.Name = newValue.Name;

	for (std::size_t j = 0; j < newValue.Values.size(); ++j)
	{
		for (int i = 0; i < glm::vec3::length(); ++i)
		{
			bone.Axes[(j * glm::vec3::length()) + i].Value = newValue.Values[j][i];
			bone.Axes[(j * glm::vec3::length()) + i].Scale = newValue.Scales[j][i];
		}
	}

	emit _asset->GetModelData()->BoneDataChanged(index);
	EmitDataChanged(_asset->GetModelData()->Bones, index);
	EmitDataChanged(_asset->GetModelData()->BonesWithNone, index + 1);
}

void ChangeAttachmentPropsCommand::Apply(int index, const AttachmentProps& oldValue, const AttachmentProps& newValue)
{
	auto model = _asset->GetEditableStudioModel();
	auto& attachment = *model->Attachments[index];
	attachment.Name = newValue.Name;
	attachment.Bone = newValue.Bone != -1 ? model->Bones[newValue.Bone].get() : nullptr;
	attachment.Origin = newValue.Origin;
	emit _asset->GetModelData()->AttachmentDataChanged(index);
	EmitDataChanged(_asset->GetModelData()->Attachments, index);
}

void ChangeBoneControllerPropsCommand::Apply(int index,
	const BoneControllerProps& oldValue, const BoneControllerProps& newValue)
{
	auto model = _asset->GetEditableStudioModel();
	auto& controller = *model->BoneControllers[index];

	controller.Start = newValue.Start;
	controller.End = newValue.End;
	controller.Rest = newValue.Rest;
	controller.Index = newValue.Index;

	auto oldBone = oldValue.Bone != -1 ? model->Bones[oldValue.Bone].get() : nullptr;
	auto newBone = newValue.Bone != -1 ? model->Bones[newValue.Bone].get() : nullptr;

	//Detach from old bone, if any
	if (oldBone)
	{
		oldBone->Axes[oldValue.BoneAxis].Controller = nullptr;
	}

	//Attach to new bone
	if (newBone)
	{
		newBone->Axes[newValue.BoneAxis].Controller = &controller;
		controller.Type = 1 << newValue.BoneAxis;
	}
	else
	{
		controller.Type = 0;
	}

	emit _asset->GetModelData()->BoneControllerDataChanged(index);
	EmitDataChanged(_asset->GetModelData()->BoneControllers, index);
}

void ChangeModelFlagsCommand::Apply(const int& oldValue, const int& newValue)
{
	_asset->GetEditableStudioModel()->Flags = newValue;
	emit _asset->GetModelData()->ModelFlagsChanged();
}

void ChangeModelOriginCommand::undo()
{
	ApplyMoveData(*_asset->GetEditableStudioModel(), _data, std::nullopt);
	emit _asset->GetModelData()->ModelOriginChanged();
}

void ChangeModelOriginCommand::redo()
{
	ApplyMoveData(*_asset->GetEditableStudioModel(), _data, _offset);
	emit _asset->GetModelData()->ModelOriginChanged();
}

void ChangeModelScaleCommand::undo()
{
	ApplyScaleData(*_asset->GetEditableStudioModel(), _data, std::nullopt);
	emit _asset->GetModelData()->ModelScaleChanged();
}

void ChangeModelScaleCommand::redo()
{
	ApplyScaleData(*_asset->GetEditableStudioModel(), _data, _scale);
	emit _asset->GetModelData()->ModelScaleChanged();
}

void ChangeModelRotationCommand::undo()
{
	ApplyRotateData(*_asset->GetEditableStudioModel(), _data, std::nullopt);
	emit _asset->GetModelData()->ModelRotationChanged();
}

void ChangeModelRotationCommand::redo()
{
	ApplyRotateData(*_asset->GetEditableStudioModel(), _data, _angles);
	emit _asset->GetModelData()->ModelRotationChanged();
}

void ChangeHitboxPropsCommand::Apply(int index, const HitboxProps& oldValue, const HitboxProps& newValue)
{
	auto model = _asset->GetEditableStudioModel();
	auto& hitbox = *model->Hitboxes[index];
	hitbox.Bone = model->Bones[newValue.Bone].get();
	hitbox.Group = newValue.Group;
	hitbox.Min = newValue.Min;
	hitbox.Max = newValue.Max;
	emit _asset->GetModelData()->HitboxDataChanged(index);
	EmitDataChanged(_asset->GetModelData()->Hitboxes, index);
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

void ChangeSequencePropsCommand::Apply(int index, const SequenceProps& oldValue, const SequenceProps& newValue)
{
	auto model = _asset->GetEditableStudioModel();
	auto& sequence = *model->Sequences[index];

	if (newValue.IsLooping)
	{
		sequence.Flags |= STUDIO_LOOPING;
	}
	else
	{
		sequence.Flags &= ~STUDIO_LOOPING;
	}

	sequence.FPS = newValue.FPS;
	sequence.Activity = newValue.Activity;
	sequence.ActivityWeight = newValue.ActivityWeight;
	sequence.LinearMovement = newValue.LinearMovement;

	EmitDataChanged(_asset->GetModelData()->Sequences, index);
}


void ChangeEventCommand::Apply(int index,
	const studiomdl::StudioSequenceEvent& oldValue, const studiomdl::StudioSequenceEvent& newValue)
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

void AddRemoveEventCommand::Add(int index, const studiomdl::StudioSequenceEvent& value)
{
	auto& sequence = *_asset->GetEditableStudioModel()->Sequences[index];

	auto event = sequence.Events.insert(sequence.Events.begin() + _eventIndex, std::make_unique<studiomdl::StudioSequenceEvent>(value))->get();
	sequence.SortedEvents.push_back(event);

	studiomdl::SortEventsList(sequence.SortedEvents);
	emit _asset->GetModelData()->EventAdded(index, _eventIndex);
}

void AddRemoveEventCommand::Remove(int index, const studiomdl::StudioSequenceEvent& value)
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

FlipNormalsCommand::FlipNormalsCommand(StudioModelAsset* asset)
	: BaseModelUndoCommand(asset, ModelChangeId::FlipNormals)
{
	setText("Flip normals");

	auto model = asset->GetEditableStudioModel();

	for (auto& bodypart : model->Bodyparts)
	{
		for (auto& model : bodypart->Models)
		{
			_normals.reserve(_normals.size() + model.Normals.size());

			for (auto& normal : model.Normals)
			{
				_normals.push_back(normal.Vertex);
			}
		}
	}
}

void FlipNormalsCommand::undo()
{
	auto model = _asset->GetEditableStudioModel();

	std::size_t normalIndex = 0;

	for (auto& bodypart : model->Bodyparts)
	{
		for (auto& model : bodypart->Models)
		{
			for (auto& normal : model.Normals)
			{
				normal.Vertex = _normals[normalIndex++];
			}
		}
	}
}

void FlipNormalsCommand::redo()
{
	auto model = _asset->GetEditableStudioModel();

	std::size_t normalIndex = 0;

	for (auto& bodypart : model->Bodyparts)
	{
		for (auto& model : bodypart->Models)
		{
			for (auto& normal : model.Normals)
			{
				normal.Vertex = -_normals[normalIndex++];
			}
		}
	}
}
}
