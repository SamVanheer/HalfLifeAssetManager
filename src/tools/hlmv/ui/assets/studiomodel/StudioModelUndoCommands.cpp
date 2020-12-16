#include "engine/shared/studiomodel/CStudioModel.h"
#include "ui/assets/studiomodel/StudioModelAsset.hpp"
#include "ui/assets/studiomodel/StudioModelUndoCommands.hpp"

namespace ui::assets::studiomodel
{
bool BoneRenameCommand::mergeWith(const QUndoCommand* other)
{
	if (id() != other->id())
	{
		return false;
	}

	_newName = static_cast<const BoneRenameCommand*>(other)->_newName;

	return true;
}

void BoneRenameCommand::undo()
{
	const auto header = _asset->GetStudioModel()->GetStudioHeader();

	const auto bone = header->GetBone(_boneIndex);

	strncpy(bone->name, _oldName.toUtf8().constData(), sizeof(bone->name) - 1);
	bone->name[sizeof(bone->name) - 1] = '\0';

	_asset->EmitModelChanged(ModelListChangeEvent{ModelChangeId::BoneRename, _boneIndex});
}

void BoneRenameCommand::redo()
{
	const auto header = _asset->GetStudioModel()->GetStudioHeader();

	const auto bone = header->GetBone(_boneIndex);

	strncpy(bone->name, _newName.toUtf8().constData(), sizeof(bone->name) - 1);
	bone->name[sizeof(bone->name) - 1] = '\0';

	_asset->EmitModelChanged(ModelListChangeEvent{ModelChangeId::BoneRename, _boneIndex});
}

bool ChangeBoneParentCommand::mergeWith(const QUndoCommand* other)
{
	if (id() != other->id())
	{
		return false;
	}

	_newParent = static_cast<const ChangeBoneParentCommand*>(other)->_newParent;

	return true;
}

void ChangeBoneParentCommand::undo()
{
	const auto header = _asset->GetStudioModel()->GetStudioHeader();

	const auto bone = header->GetBone(_boneIndex);

	bone->parent = _oldParent;

	_asset->EmitModelChanged(ModelListChangeEvent{ModelChangeId::ChangeBoneParent, _boneIndex});
}

void ChangeBoneParentCommand::redo()
{
	const auto header = _asset->GetStudioModel()->GetStudioHeader();

	const auto bone = header->GetBone(_boneIndex);

	bone->parent = _newParent;

	_asset->EmitModelChanged(ModelListChangeEvent{ModelChangeId::ChangeBoneParent, _boneIndex});
}

bool ChangeBoneFlagsCommand::mergeWith(const QUndoCommand* other)
{
	if (id() != other->id())
	{
		return false;
	}

	_newFlags = static_cast<const ChangeBoneFlagsCommand*>(other)->_newFlags;

	return true;
}

void ChangeBoneFlagsCommand::undo()
{
	const auto header = _asset->GetStudioModel()->GetStudioHeader();

	const auto bone = header->GetBone(_boneIndex);

	bone->flags = _oldFlags;

	_asset->EmitModelChanged(ModelListChangeEvent{ModelChangeId::ChangeBoneFlags, _boneIndex});
}

void ChangeBoneFlagsCommand::redo()
{
	const auto header = _asset->GetStudioModel()->GetStudioHeader();

	const auto bone = header->GetBone(_boneIndex);

	bone->flags = _newFlags;

	_asset->EmitModelChanged(ModelListChangeEvent{ModelChangeId::ChangeBoneFlags, _boneIndex});
}

bool ChangeBonePropertyCommand::mergeWith(const QUndoCommand* other)
{
	if (id() != other->id())
	{
		return false;
	}

	_newProperties = static_cast<const ChangeBonePropertyCommand*>(other)->_newProperties;

	return true;
}

static constexpr void CopyProperties(mstudiobone_t& bone, const ChangeBonePropertyCommand::Properties& properties)
{
	for (std::size_t j = 0; j < properties.Values.size(); ++j)
	{
		for (int i = 0; i < properties.Values[j].length(); ++i)
		{
			bone.value[(j * properties.Values[j].length()) + i] = properties.Values[j][i];
			bone.scale[(j * properties.Scales[j].length()) + i] = properties.Scales[j][i];
		}
	}
}

void ChangeBonePropertyCommand::undo()
{
	const auto header = _asset->GetStudioModel()->GetStudioHeader();

	const auto bone = header->GetBone(_boneIndex);

	CopyProperties(*bone, _oldProperties);

	_asset->EmitModelChanged(ModelListChangeEvent{ModelChangeId::ChangeBoneProperty, _boneIndex});
}

void ChangeBonePropertyCommand::redo()
{
	const auto header = _asset->GetStudioModel()->GetStudioHeader();

	const auto bone = header->GetBone(_boneIndex);

	CopyProperties(*bone, _newProperties);

	_asset->EmitModelChanged(ModelListChangeEvent{ModelChangeId::ChangeBoneProperty, _boneIndex});
}
}
