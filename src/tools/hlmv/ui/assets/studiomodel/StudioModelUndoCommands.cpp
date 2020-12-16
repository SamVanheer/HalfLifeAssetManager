#include "engine/shared/studiomodel/CStudioModel.h"
#include "ui/assets/studiomodel/StudioModelAsset.hpp"
#include "ui/assets/studiomodel/StudioModelUndoCommands.hpp"

namespace ui::assets::studiomodel
{
bool ModelBoneRenameCommand::mergeWith(const QUndoCommand* other)
{
	if (id() != other->id())
	{
		return false;
	}

	//The new name is whatever the latest rename command says it is
	_newName = static_cast<const ModelBoneRenameCommand*>(other)->_newName;

	return true;
}

void ModelBoneRenameCommand::undo()
{
	const auto header = _asset->GetStudioModel()->GetStudioHeader();

	const auto bone = header->GetBone(_boneIndex);

	strncpy(bone->name, _oldName.toUtf8().constData(), sizeof(bone->name) - 1);
	bone->name[sizeof(bone->name) - 1] = '\0';

	_asset->EmitModelChanged(ModelListChangeEvent{ModelChangeId::BoneRename, _boneIndex});
}

void ModelBoneRenameCommand::redo()
{
	const auto header = _asset->GetStudioModel()->GetStudioHeader();

	const auto bone = header->GetBone(_boneIndex);

	strncpy(bone->name, _newName.toUtf8().constData(), sizeof(bone->name) - 1);
	bone->name[sizeof(bone->name) - 1] = '\0';

	_asset->EmitModelChanged(ModelListChangeEvent{ModelChangeId::BoneRename, _boneIndex});
}
}
