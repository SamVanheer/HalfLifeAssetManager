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
}
