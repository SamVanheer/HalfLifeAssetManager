#include <cmath>

#include "engine/shared/studiomodel/StudioModel.hpp"
#include "entity/HLMVStudioModelEntity.hpp"
#include "ui/assets/studiomodel/StudioModelAsset.hpp"
#include "ui/assets/studiomodel/StudioModelUndoCommands.hpp"

namespace ui::assets::studiomodel
{
void ChangeEyePositionCommand::Apply(const glm::vec3& oldValue, const glm::vec3& newValue)
{
	const auto header = _asset->GetStudioModel()->GetStudioHeader();
	header->eyeposition = newValue;
}

void ChangeBBoxCommand::Apply(const std::pair<glm::vec3, glm::vec3>& oldValue, const std::pair<glm::vec3, glm::vec3>& newValue)
{
	const auto header = _asset->GetStudioModel()->GetStudioHeader();
	header->min = newValue.first;
	header->max = newValue.second;
}

void ChangeCBoxCommand::Apply(const std::pair<glm::vec3, glm::vec3>& oldValue, const std::pair<glm::vec3, glm::vec3>& newValue)
{
	const auto header = _asset->GetStudioModel()->GetStudioHeader();
	header->bbmin = newValue.first;
	header->bbmax = newValue.second;
}

void BoneRenameCommand::Apply(int index, const QString& oldValue, const QString& newValue)
{
	const auto header = _asset->GetStudioModel()->GetStudioHeader();
	const auto bone = header->GetBone(index);

	strncpy(bone->name, newValue.toUtf8().constData(), sizeof(bone->name) - 1);
	bone->name[sizeof(bone->name) - 1] = '\0';
}

void ChangeBoneParentCommand::Apply(int index, const int& oldValue, const int& newValue)
{
	const auto header = _asset->GetStudioModel()->GetStudioHeader();
	const auto bone = header->GetBone(index);

	bone->parent = newValue;
}

void ChangeBoneFlagsCommand::Apply(int index, const int& oldValue, const int& newValue)
{
	const auto header = _asset->GetStudioModel()->GetStudioHeader();
	const auto bone = header->GetBone(index);

	bone->flags = newValue;
}

void ChangeBonePropertyCommand::Apply(int index, const ChangeBoneProperties& oldValue, const ChangeBoneProperties& newValue)
{
	const auto header = _asset->GetStudioModel()->GetStudioHeader();
	const auto bone = header->GetBone(index);

	for (std::size_t j = 0; j < newValue.Values.size(); ++j)
	{
		for (int i = 0; i < newValue.Values[j].length(); ++i)
		{
			bone->value[(j * newValue.Values[j].length()) + i] = newValue.Values[j][i];
			bone->scale[(j * newValue.Scales[j].length()) + i] = newValue.Scales[j][i];
		}
	}
}

void ChangeAttachmentNameCommand::Apply(int index, const QString& oldValue, const QString& newValue)
{
	const auto header = _asset->GetStudioModel()->GetStudioHeader();
	const auto attachment = header->GetAttachment(index);

	strncpy(attachment->name, newValue.toUtf8().constData(), sizeof(attachment->name) - 1);
	attachment->name[sizeof(attachment->name) - 1] = '\0';
}

void ChangeAttachmentTypeCommand::Apply(int index, const int& oldValue, const int& newValue)
{
	const auto header = _asset->GetStudioModel()->GetStudioHeader();
	const auto attachment = header->GetAttachment(index);

	attachment->type = newValue;
}

void ChangeAttachmentBoneCommand::Apply(int index, const int& oldValue, const int& newValue)
{
	const auto header = _asset->GetStudioModel()->GetStudioHeader();
	const auto attachment = header->GetAttachment(index);

	attachment->bone = newValue;
}

void ChangeAttachmentOriginCommand::Apply(int index, const glm::vec3& oldValue, const glm::vec3& newValue)
{
	const auto header = _asset->GetStudioModel()->GetStudioHeader();
	const auto attachment = header->GetAttachment(index);

	for (int i = 0; i < newValue.length(); ++i)
	{
		attachment->org[i] = newValue[i];
	}
}

void ChangeBoneControllerBoneCommand::Apply(int index, const int& oldValue, const int& newValue)
{
	const auto header = _asset->GetStudioModel()->GetStudioHeader();
	const auto controller = header->GetBoneController(index);

	const int type = controller->type & STUDIO_BONECONTROLLER_TYPES;

	const int typeIndex = static_cast<int>(std::log2(type));

	const auto oldBone = header->GetBone(oldValue);
	const auto newBone = header->GetBone(newValue);

	//Remove the reference to this controller from the old bone
	oldBone->bonecontroller[typeIndex] = -1;

	controller->bone = newValue;

	//Patch up the new bone reference
	newBone->bonecontroller[typeIndex] = index;
}

void ChangeBoneControllerRangeCommand::Apply(int index, const ChangeBoneControllerRange& oldValue, const ChangeBoneControllerRange& newValue)
{
	const auto header = _asset->GetStudioModel()->GetStudioHeader();
	const auto controller = header->GetBoneController(index);

	controller->start = newValue.Start;
	controller->end = newValue.End;
}

void ChangeBoneControllerRestCommand::Apply(int index, const int& oldValue, const int& newValue)
{
	const auto header = _asset->GetStudioModel()->GetStudioHeader();
	const auto controller = header->GetBoneController(index);

	controller->rest = newValue;
}

void ChangeBoneControllerIndexCommand::Apply(int index, const int& oldValue, const int& newValue)
{
	const auto header = _asset->GetStudioModel()->GetStudioHeader();
	const auto controller = header->GetBoneController(index);

	controller->index = newValue;
}

void ChangeBoneControllerTypeCommand::Apply(int index, const int& oldValue, const int& newValue)
{
	const auto header = _asset->GetStudioModel()->GetStudioHeader();
	const auto controller = header->GetBoneController(index);
	const auto bone = header->GetBone(controller->bone);

	const int oldTypeIndex = static_cast<int>(std::log2(oldValue));
	const int newTypeIndex = static_cast<int>(std::log2(newValue));

	bone->bonecontroller[oldTypeIndex] = -1;

	controller->type &= ~oldValue;
	controller->type |= newValue;

	bone->bonecontroller[newTypeIndex] = index;
}

void ChangeModelFlagsCommand::Apply(const int& oldValue, const int& newValue)
{
	const auto header = _asset->GetStudioModel()->GetStudioHeader();

	header->flags = newValue;
}

void ChangeModelOriginCommand::Apply(const ChangeModelOriginData& oldValue, const ChangeModelOriginData& newValue)
{
	const auto header = _asset->GetStudioModel()->GetStudioHeader();

	for (const auto& data : newValue.BoneData)
	{
		const auto bone = header->GetBone(data.BoneIndex);

		for (int i = 0; i < data.BonePosition.length(); ++i)
		{
			bone->value[i] = data.BonePosition[i];
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
	const auto header = _asset->GetStudioModel()->GetStudioHeader();
	const auto hitbox = header->GetHitBox(index);

	hitbox->bone = newValue;
}

void ChangeHitboxHitgroupCommand::Apply(int index, const int& oldValue, const int& newValue)
{
	const auto header = _asset->GetStudioModel()->GetStudioHeader();
	const auto hitbox = header->GetHitBox(index);

	hitbox->group = newValue;
}

void ChangeHitboxBoundsCommand::Apply(int index, const std::pair<glm::vec3, glm::vec3>& oldValue, const std::pair<glm::vec3, glm::vec3>& newValue)
{
	const auto header = _asset->GetStudioModel()->GetStudioHeader();
	const auto hitbox = header->GetHitBox(index);

	hitbox->bbmin = newValue.first;
	hitbox->bbmax = newValue.second;
}

void ChangeTextureNameCommand::Apply(int index, const QString& oldValue, const QString& newValue)
{
	const auto header = _asset->GetStudioModel()->GetTextureHeader();
	const auto texture = header->GetTexture(index);

	strncpy(texture->name, newValue.toStdString().c_str(), sizeof(texture->name) - 1);
	texture->name[sizeof(texture->name) - 1] = '\0';
}

void ChangeTextureFlagsCommand::Apply(int index, const int& oldValue, const int& newValue)
{
	const auto header = _asset->GetStudioModel()->GetTextureHeader();
	const auto texture = header->GetTexture(index);

	texture->flags = newValue;
}

void ImportTextureCommand::Apply(int index, const ImportTextureData& oldValue, const ImportTextureData& newValue)
{
	const auto model = _asset->GetStudioModel();
	const auto header = model->GetTextureHeader();
	const auto texture = header->GetTexture(index);

	//Copy over the new image data to the texture
	memcpy(header->GetData() + texture->index, newValue.Pixels.get(), newValue.Width * newValue.Height);
	memcpy(header->GetData() + texture->index + (newValue.Width * newValue.Height), newValue.Palette, PALETTE_SIZE);

	model->ReplaceTexture(*_asset->GetTextureLoader(), texture, newValue.Pixels.get(), newValue.Palette, model->GetTextureId(index));
}

void ChangeEventCommand::Apply(int index, const studiomdl::SequenceEvent& oldValue, const studiomdl::SequenceEvent& newValue)
{
	auto model = _asset->GetEditableStudioModel();
	auto& sequence = *model->Sequences[index];

	sequence.Events[_eventIndex] = newValue;
}

void ChangeModelNameCommand::Apply(int index, const QString& oldValue, const QString& newValue)
{
	const auto header = _asset->GetStudioModel()->GetStudioHeader();
	const auto bodyPart = header->GetBodypart(index);
	const auto model = reinterpret_cast<mstudiomodel_t*>(header->GetData() + bodyPart->modelindex) + _modelIndex;

	strncpy(model->name, newValue.toStdString().c_str(), sizeof(model->name) - 1);
	model->name[sizeof(model->name) - 1] = '\0';
}
}
