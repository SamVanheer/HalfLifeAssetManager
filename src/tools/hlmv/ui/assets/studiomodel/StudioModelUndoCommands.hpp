#pragma once

#include <array>
#include <cassert>

#include <QString>
#include <QUndoStack>

#include <glm/vec3.hpp>

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
	ChangeBoneParent,
	ChangeBoneFlags,
	ChangeBoneProperty,
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

class BoneRenameCommand : public ModelUndoCommand
{
public:
	BoneRenameCommand(StudioModelAsset* asset, int boneIndex, QString&& oldName, QString&& newName)
		: ModelUndoCommand(asset, ModelChangeId::BoneRename)
		, _boneIndex(boneIndex)
		, _oldName(std::move(oldName))
		, _newName(std::move(newName))
	{
		setText("Rename bone");
	}

	bool mergeWith(const QUndoCommand* other) override;

	void undo() override;
	void redo() override;

private:
	const int _boneIndex;
	const QString _oldName;
	QString _newName;
};

class ChangeBoneParentCommand : public ModelUndoCommand
{
public:
	ChangeBoneParentCommand(StudioModelAsset* asset, int boneIndex, int oldParent, int newParent)
		: ModelUndoCommand(asset, ModelChangeId::ChangeBoneParent)
		, _boneIndex(boneIndex)
		, _oldParent(oldParent)
		, _newParent(newParent)
	{
		setText("Change bone parent");
	}

	bool mergeWith(const QUndoCommand* other) override;

	void undo() override;
	void redo() override;

private:
	const int _boneIndex;
	const int _oldParent;
	int _newParent;
};

class ChangeBoneFlagsCommand : public ModelUndoCommand
{
public:
	ChangeBoneFlagsCommand(StudioModelAsset* asset, int boneIndex, int oldFlags, int newFlags)
		: ModelUndoCommand(asset, ModelChangeId::ChangeBoneParent)
		, _boneIndex(boneIndex)
		, _oldFlags(oldFlags)
		, _newFlags(newFlags)
	{
		setText("Change bone flags");
	}

	bool mergeWith(const QUndoCommand* other) override;

	void undo() override;
	void redo() override;

private:
	const int _boneIndex;
	const int _oldFlags;
	int _newFlags;
};

class ChangeBonePropertyCommand : public ModelUndoCommand
{
public:
	struct Properties
	{
		std::array<glm::vec3, 2> Values;
		std::array<glm::vec3, 2> Scales;
	};

	ChangeBonePropertyCommand(StudioModelAsset* asset, int boneIndex, const Properties& oldProperties, const Properties& newProperties)
		: ModelUndoCommand(asset, ModelChangeId::ChangeBoneProperty)
		, _boneIndex(boneIndex)
		, _oldProperties(oldProperties)
		, _newProperties(newProperties)
	{
		setText("Change bone property");
	}

	bool mergeWith(const QUndoCommand* other) override;

	void undo() override;
	void redo() override;

private:
	const int _boneIndex;
	const Properties _oldProperties;
	Properties _newProperties;
};
}
