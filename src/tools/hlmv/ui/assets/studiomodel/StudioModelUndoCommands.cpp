#include "engine/shared/studiomodel/CStudioModel.h"
#include "ui/assets/studiomodel/StudioModelAsset.hpp"
#include "ui/assets/studiomodel/StudioModelUndoCommands.hpp"

namespace ui::assets::studiomodel
{
void BoneRenameCommand::Apply(int index, const QString& value)
{
	const auto header = _asset->GetStudioModel()->GetStudioHeader();
	const auto bone = header->GetBone(index);

	strncpy(bone->name, value.toUtf8().constData(), sizeof(bone->name) - 1);
	bone->name[sizeof(bone->name) - 1] = '\0';
}

void ChangeBoneParentCommand::Apply(int index, const int& value)
{
	const auto header = _asset->GetStudioModel()->GetStudioHeader();
	const auto bone = header->GetBone(index);

	bone->parent = value;
}

void ChangeBoneFlagsCommand::Apply(int index, const int& value)
{
	const auto header = _asset->GetStudioModel()->GetStudioHeader();
	const auto bone = header->GetBone(index);

	bone->flags = value;
}

void ChangeBonePropertyCommand::Apply(int index, const ChangeBoneProperties& value)
{
	const auto header = _asset->GetStudioModel()->GetStudioHeader();
	const auto bone = header->GetBone(index);

	for (std::size_t j = 0; j < value.Values.size(); ++j)
	{
		for (int i = 0; i < value.Values[j].length(); ++i)
		{
			bone->value[(j * value.Values[j].length()) + i] = value.Values[j][i];
			bone->scale[(j * value.Scales[j].length()) + i] = value.Scales[j][i];
		}
	}
}

void ChangeAttachmentNameCommand::Apply(int index, const QString& value)
{
	const auto header = _asset->GetStudioModel()->GetStudioHeader();
	const auto attachment = header->GetAttachment(index);

	strncpy(attachment->name, value.toUtf8().constData(), sizeof(attachment->name) - 1);
	attachment->name[sizeof(attachment->name) - 1] = '\0';
}

void ChangeAttachmentTypeCommand::Apply(int index, const int& value)
{
	const auto header = _asset->GetStudioModel()->GetStudioHeader();
	const auto attachment = header->GetAttachment(index);

	attachment->type = value;
}

void ChangeAttachmentBoneCommand::Apply(int index, const int& value)
{
	const auto header = _asset->GetStudioModel()->GetStudioHeader();
	const auto attachment = header->GetAttachment(index);

	attachment->bone = value;
}

void ChangeAttachmentOriginCommand::Apply(int index, const glm::vec3& value)
{
	const auto header = _asset->GetStudioModel()->GetStudioHeader();
	const auto attachment = header->GetAttachment(index);

	for (int i = 0; i < value.length(); ++i)
	{
		attachment->org[i] = value[i];
	}
}
}