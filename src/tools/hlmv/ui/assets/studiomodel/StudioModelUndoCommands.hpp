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

enum class ModelChangeId
{
	BoneRename,
};

/**
*	@brief Base class for all model change events
*	Indicates that a change has been made to a model
*/
class ModelChangeEvent
{
public:
	ModelChangeEvent(ModelChangeId id)
		: _id(id)
	{
	}

	ModelChangeEvent(const ModelChangeEvent&) = delete;
	ModelChangeEvent& operator=(const ModelChangeEvent&) = delete;

	ModelChangeId GetId() const { return _id; }

private:
	const ModelChangeId _id;
};

/**
*	@brief Base class for all model change events that involve a change that occurred in a list
*/
class ModelListChangeEvent : public ModelChangeEvent
{
public:
	ModelListChangeEvent(ModelChangeId id, int sourceIndex, int destinationIndex = -1)
		: ModelChangeEvent(id)
		, _sourceIndex(sourceIndex)
		, _destinationIndex(destinationIndex)
	{
	}

	/**
	*	@brief The list entry being changed. If this change involves the moving of a list entry, this is where the entry was moved from
	*/
	int GetSourceIndex() const { return _sourceIndex; }

	/**
	*	@brief If this change involves the moving of a list entry, this is where the entry was moved to
	*/
	int GetDestinationIndex() const { return _destinationIndex; }

private:
	const int _sourceIndex;
	const int _destinationIndex;
};

/**
*	@brief Base class for all undo commands related to Studiomodel editing
*/
class ModelUndoCommand : public QUndoCommand
{
public:
	ModelUndoCommand(StudioModelAsset* asset, ModelChangeId id)
		: _asset(asset)
		, _id(id)
	{
		assert(_asset);
	}

	int id() const override final { return static_cast<int>(_id); }

protected:
	StudioModelAsset* const _asset;

private:
	const ModelChangeId _id;
};

class ModelBoneRenameCommand : public ModelUndoCommand
{
public:
	ModelBoneRenameCommand(StudioModelAsset* asset, int boneIndex, QString&& oldName, QString&& newName)
		: ModelUndoCommand(asset, ModelChangeId::BoneRename)
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
