#pragma once

#include <array>
#include <cassert>

#include <QString>
#include <QUndoStack>

#include <glm/vec3.hpp>

#include "ui/assets/studiomodel/StudioModelAsset.hpp"

namespace studiomdl
{
class CStudioModel;
}

namespace ui::assets::studiomodel
{
enum class ModelChangeId
{
	RenameBone,
	ChangeBoneParent,
	ChangeBoneFlags,
	ChangeBoneProperty,

	ChangeAttachmentName,
	ChangeAttachmentType,
	ChangeAttachmentBone,
	ChangeAttachmentOrigin,
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
protected:
	ModelUndoCommand(StudioModelAsset* asset, ModelChangeId id)
		: _asset(asset)
		, _id(id)
	{
		assert(_asset);
	}

public:
	int id() const override final { return static_cast<int>(_id); }

protected:
	StudioModelAsset* const _asset;

protected:
	const ModelChangeId _id;
};

template<typename T>
class ModelListUndoCommand : public ModelUndoCommand
{
protected:
	ModelListUndoCommand(StudioModelAsset* asset, ModelChangeId id, int index, const T& oldValue, const T& newValue)
		: ModelUndoCommand(asset, id)
		, _index(index)
		, _oldValue(oldValue)
		, _newValue(newValue)
	{
	}

public:
	bool mergeWith(const QUndoCommand* other) override
	{
		if (id() != other->id())
		{
			return false;
		}

		_newValue = static_cast<const ModelListUndoCommand*>(other)->_newValue;

		return true;
	}

	void undo() override
	{
		Apply(_index, _oldValue);
		_asset->EmitModelChanged(ModelListChangeEvent{_id, _index});
	}

	void redo() override
	{
		Apply(_index, _newValue);
		_asset->EmitModelChanged(ModelListChangeEvent{_id, _index});
	}

protected:
	virtual void Apply(int index, const T& value) = 0;

protected:
	const int _index;
	const T _oldValue;
	T _newValue;
};

class BoneRenameCommand : public ModelListUndoCommand<QString>
{
public:
	BoneRenameCommand(StudioModelAsset* asset, int boneIndex, const QString& oldName, const QString& newName)
		: ModelListUndoCommand(asset, ModelChangeId::RenameBone, boneIndex, oldName, newName)
	{
		setText("Rename bone");
	}

protected:
	void Apply(int index, const QString& value) override;
};

class ChangeBoneParentCommand : public ModelListUndoCommand<int>
{
public:
	ChangeBoneParentCommand(StudioModelAsset* asset, int boneIndex, int oldParent, int newParent)
		: ModelListUndoCommand(asset, ModelChangeId::ChangeBoneParent, boneIndex, oldParent, newParent)
	{
		setText("Change bone parent");
	}

protected:
	void Apply(int index, const int& value) override;
};

class ChangeBoneFlagsCommand : public ModelListUndoCommand<int>
{
public:
	ChangeBoneFlagsCommand(StudioModelAsset* asset, int boneIndex, int oldFlags, int newFlags)
		: ModelListUndoCommand(asset, ModelChangeId::ChangeBoneParent, boneIndex, oldFlags, newFlags)
	{
		setText("Change bone flags");
	}

protected:
	void Apply(int index, const int& value) override;
};

struct ChangeBoneProperties
{
	std::array<glm::vec3, 2> Values;
	std::array<glm::vec3, 2> Scales;
};

class ChangeBonePropertyCommand : public ModelListUndoCommand<ChangeBoneProperties>
{
public:
	ChangeBonePropertyCommand(StudioModelAsset* asset, int boneIndex, const ChangeBoneProperties& oldProperties, const ChangeBoneProperties& newProperties)
		: ModelListUndoCommand(asset, ModelChangeId::ChangeBoneProperty, boneIndex, oldProperties, newProperties)
	{
		setText("Change bone property");
	}

protected:
	void Apply(int index, const ChangeBoneProperties& properties) override;
};

class ChangeAttachmentNameCommand : public ModelListUndoCommand<QString>
{
public:
	ChangeAttachmentNameCommand(StudioModelAsset* asset, int attachmentIndex, const QString& oldName, const QString& newName)
		: ModelListUndoCommand(asset, ModelChangeId::ChangeAttachmentName, attachmentIndex, oldName, newName)
	{
		setText("Change attachment name");
	}

protected:
	void Apply(int index, const QString& value) override;
};

class ChangeAttachmentTypeCommand : public ModelListUndoCommand<int>
{
public:
	ChangeAttachmentTypeCommand(StudioModelAsset* asset, int attachmentIndex, int oldType, int newType)
		: ModelListUndoCommand(asset, ModelChangeId::ChangeAttachmentType, attachmentIndex, oldType, newType)
	{
		setText("Change attachment type");
	}

protected:
	void Apply(int index, const int& value) override;
};

class ChangeAttachmentBoneCommand : public ModelListUndoCommand<int>
{
public:
	ChangeAttachmentBoneCommand(StudioModelAsset* asset, int attachmentIndex, int oldBone, int newBone)
		: ModelListUndoCommand(asset, ModelChangeId::ChangeAttachmentBone, attachmentIndex, oldBone, newBone)
	{
		setText("Change attachment bone");
	}

protected:
	void Apply(int index, const int& value) override;
};

class ChangeAttachmentOriginCommand : public ModelListUndoCommand<glm::vec3>
{
public:
	ChangeAttachmentOriginCommand(StudioModelAsset* asset, int attachmentIndex, const glm::vec3& oldOrigin, const glm::vec3& newOrigin)
		: ModelListUndoCommand(asset, ModelChangeId::ChangeAttachmentOrigin, attachmentIndex, oldOrigin, newOrigin)
	{
		setText("Change attachment origin");
	}

protected:
	void Apply(int index, const glm::vec3& value) override;
};
}
