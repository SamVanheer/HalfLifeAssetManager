#pragma once

#include <cassert>

#include <QString>
#include <QUndoStack>

namespace studiomdl
{
class CStudioModel;
}

namespace ui::assets::studiomodel
{
class StudioModelAsset;

enum class ModelUndoId
{
	BoneRename,
};

/**
*	@brief Base class for all undo commands related to Studiomodel editing
*/
class ModelUndoCommand : public QUndoCommand
{
public:
	ModelUndoCommand(studiomdl::CStudioModel* model, ModelUndoId id)
		: _model(model)
		, _id(id)
	{
		assert(_model);
	}

	int id() const override final { return static_cast<int>(_id); }

protected:
	studiomdl::CStudioModel* const _model;

private:
	const ModelUndoId _id;
};

class ModelBoneRenameCommand : public ModelUndoCommand
{
public:
	ModelBoneRenameCommand(studiomdl::CStudioModel* model, int boneIndex, QString&& oldName, QString&& newName)
		: ModelUndoCommand(model, ModelUndoId::BoneRename)
		, _boneIndex(boneIndex)
		, _oldName(std::move(oldName))
		, _newName(std::move(newName))
	{
		setText("Rename bone");
	}

	int GetBoneIndex() const { return _boneIndex; }

	QString GetOldName() const { return _oldName; }

	QString GetNewName() const { return _newName; }

	bool mergeWith(const QUndoCommand* other) override;

	void undo() override;
	void redo() override;

private:
	const int _boneIndex;
	const QString _oldName;
	QString _newName;
};
}
