#pragma once

#include <array>
#include <cassert>
#include <cstring>
#include <memory>
#include <vector>

#include <QString>
#include <QUndoStack>

#include <glm/vec3.hpp>

#include "core/shared/Const.hpp"
#include "engine/shared/studiomodel/StudioModelFileFormat.hpp"
#include "graphics/Palette.hpp"

#include "ui/assets/studiomodel/StudioModelAsset.hpp"

namespace studiomdl
{
struct ScaleBonesBoneData;
struct ScaleMeshesData;
}

namespace ui::assets::studiomodel
{
enum class ModelChangeId
{
	ChangeEyePosition,
	ChangeBBox,
	ChangeCBox,

	RenameBone,
	ChangeBoneParent,
	ChangeBoneFlags,
	ChangeBoneProperty,

	ChangeAttachmentName,
	ChangeAttachmentType,
	ChangeAttachmentBone,
	ChangeAttachmentOrigin,

	ChangeBoneControllerBone,
	ChangeBoneControllerRange,
	ChangeBoneControllerRest,
	ChangeBoneControllerIndex,
	ChangeBoneControllerType,

	ChangeModelFlags,
	ChangeModelOrigin,
	ChangeModelMeshesScale,
	ChangeModelBonesScale,

	ChangeHitboxBone,
	ChangeHitboxHitgroup,
	ChangeHitboxBounds,

	ChangeTextureName,
	ChangeTextureFlags,
	ImportTexture,

	ChangeEvent,

	ChangeModelName,
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
*	@brief Base class for model change events that have a single (compound) value
*/
template<typename T>
class ModelValueChangeEvent : public ModelChangeEvent
{
public:
	ModelValueChangeEvent(ModelChangeId id, const T& value)
		: ModelChangeEvent(id)
		, _value(value)
	{
	}

	const T& GetValue() const { return _value; }

private:
	const T _value;
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
*	@brief Base class for all model change events that involve a change that occurred in a list inside another list
*/
class ModelListSubListChangeEvent : public ModelListChangeEvent
{
public:
	ModelListSubListChangeEvent(ModelChangeId id, int sourceIndex, int destinationIndex, int sourceSubIndex, int destinationSubIndex)
		: ModelListChangeEvent(id, sourceIndex, destinationIndex)
		, _sourceSubIndex(sourceSubIndex)
		, _destinationSubIndex(destinationSubIndex)
	{
	}

	/**
	*	@brief The list entry being changed. If this change involves the moving of a list entry, this is where the entry was moved from
	*/
	int GetSourceSubIndex() const { return _sourceSubIndex; }

	/**
	*	@brief If this change involves the moving of a list entry, this is where the entry was moved to
	*/
	int GetDestinationSubIndex() const { return _destinationSubIndex; }

private:
	const int _sourceSubIndex;
	const int _destinationSubIndex;
};

class ModelEyePositionChangeEvent : public ModelChangeEvent
{
public:
	ModelEyePositionChangeEvent(ModelChangeId id, const glm::vec3& position)
		: ModelChangeEvent(id)
		, _position(position)
	{
	}

	const glm::vec3& GetPosition() const { return _position; }

private:
	const glm::vec3 _position;
};

using ModelBBoxChangeEvent = ModelValueChangeEvent<std::pair<glm::vec3, glm::vec3>>;
using ModelCBoxChangeEvent = ModelValueChangeEvent<std::pair<glm::vec3, glm::vec3>>;

class ModelOriginChangeEvent : public ModelChangeEvent
{
public:
	ModelOriginChangeEvent(ModelChangeId id, const glm::vec3& offset)
		: ModelChangeEvent(id)
		, _offset(offset)
	{
	}

	const glm::vec3& GetOffset() const { return _offset; }

private:
	const glm::vec3 _offset;
};

class ModelEventChangeEvent : public ModelListChangeEvent
{
public:
	ModelEventChangeEvent(ModelChangeId id, int sourceIndex, int eventIndex)
		: ModelListChangeEvent(id, sourceIndex)
		, _eventIndex(eventIndex)
	{
	}

	int GetEventIndex() const { return _eventIndex; }

private:
	const int _eventIndex;
};

/**
*	@brief Base class for all undo commands related to Studiomodel editing
*/
class BaseModelUndoCommand : public QUndoCommand
{
protected:
	BaseModelUndoCommand(StudioModelAsset* asset, ModelChangeId id)
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
class ModelUndoCommand : public BaseModelUndoCommand
{
protected:
	ModelUndoCommand(StudioModelAsset* asset, ModelChangeId id, const T& oldValue, const T& newValue)
		: BaseModelUndoCommand(asset, id)
		, _oldValue(oldValue)
		, _newValue(newValue)
	{
	}

	ModelUndoCommand(StudioModelAsset* asset, ModelChangeId id, T&& oldValue, T&& newValue)
		: BaseModelUndoCommand(asset, id)
		, _oldValue(std::move(oldValue))
		, _newValue(std::move(newValue))
	{
	}

public:
	bool mergeWith(const QUndoCommand* other) override
	{
		if (id() != other->id())
		{
			return false;
		}

		const auto otherUndo = static_cast<const ModelUndoCommand*>(other);

		//Don't merge if the old and new are the same
		if (!CanMerge(otherUndo))
		{
			return false;
		}

		_newValue = otherUndo->_newValue;

		return true;
	}

	void undo() override
	{
		Apply(_newValue, _oldValue);
		EmitEvent(_newValue, _oldValue);
	}

	void redo() override
	{
		Apply(_oldValue, _newValue);
		EmitEvent(_oldValue, _newValue);
	}

	const T& GetOldValue() const { return _oldValue; }

	const T& GetNewValue() const { return _newValue; }

protected:
	virtual bool CanMerge(const ModelUndoCommand* other) { return false; }

	virtual void Apply(const T& oldValue, const T& newValue) = 0;

	virtual void EmitEvent(const T& oldValue, const T& newValue)
	{
		_asset->EmitModelChanged(ModelChangeEvent{_id});
	}

protected:
	const T _oldValue;
	T _newValue;
};

template<typename T>
class ModelListUndoCommand : public BaseModelUndoCommand
{
protected:
	ModelListUndoCommand(StudioModelAsset* asset, ModelChangeId id, int index, const T& oldValue, const T& newValue)
		: BaseModelUndoCommand(asset, id)
		, _index(index)
		, _oldValue(oldValue)
		, _newValue(newValue)
	{
	}

	ModelListUndoCommand(StudioModelAsset* asset, ModelChangeId id, int index, T&& oldValue, T&& newValue)
		: BaseModelUndoCommand(asset, id)
		, _index(index)
		, _oldValue(std::move(oldValue))
		, _newValue(std::move(newValue))
	{
	}

public:
	bool mergeWith(const QUndoCommand* other) override
	{
		if (id() != other->id())
		{
			return false;
		}

		const auto otherUndo = static_cast<const ModelListUndoCommand*>(other);

		//Didn't change the same list element
		if (_index != otherUndo->_index)
		{
			return false;
		}

		if (!CanMerge(otherUndo))
		{
			return false;
		}

		_newValue = otherUndo->_newValue;

		return true;
	}

	void undo() override
	{
		Apply(_index, _newValue, _oldValue);
		EmitEvent(_newValue, _oldValue);
	}

	void redo() override
	{
		Apply(_index, _oldValue, _newValue);
		EmitEvent(_oldValue, _newValue);
	}

	int GetIndex() const { return _index; }

	const T& GetOldValue() const { return _oldValue; }

	const T& GetNewValue() const { return _newValue; }

protected:
	virtual bool CanMerge(const ModelListUndoCommand* other) { return false; }

	virtual void Apply(int index, const T& oldValue, const T& newValue) = 0;

	virtual void EmitEvent(const T& oldValue, const T& newValue)
	{
		_asset->EmitModelChanged(ModelListChangeEvent{_id, _index});
	}

protected:
	const int _index;
	const T _oldValue;
	T _newValue;
};

class ChangeEyePositionCommand : public ModelUndoCommand<glm::vec3>
{
public:
	ChangeEyePositionCommand(StudioModelAsset* asset, const glm::vec3& oldEyePosition, const glm::vec3& newEyePosition)
		: ModelUndoCommand(asset, ModelChangeId::ChangeEyePosition, oldEyePosition, newEyePosition)
	{
		setText("Change eye position");
	}

protected:
	bool CanMerge(const ModelUndoCommand<glm::vec3>* other) override
	{
		return _oldValue != other->GetNewValue();
	}

	void Apply(const glm::vec3& oldValue, const glm::vec3& newValue) override;

	void EmitEvent(const glm::vec3& oldValue, const glm::vec3& newValue) override
	{
		_asset->EmitModelChanged(ModelEyePositionChangeEvent{_id, newValue});
	}
};

class ChangeBBoxCommand : public ModelUndoCommand<std::pair<glm::vec3, glm::vec3>>
{
public:
	ChangeBBoxCommand(StudioModelAsset* asset, const std::pair<glm::vec3, glm::vec3>& oldEyePosition, const std::pair<glm::vec3, glm::vec3>& newEyePosition)
		: ModelUndoCommand(asset, ModelChangeId::ChangeBBox, oldEyePosition, newEyePosition)
	{
		setText("Change bounding box");
	}

protected:
	bool CanMerge(const ModelUndoCommand<std::pair<glm::vec3, glm::vec3>>* other) override
	{
		return _oldValue != other->GetNewValue();
	}

	void Apply(const std::pair<glm::vec3, glm::vec3>& oldValue, const std::pair<glm::vec3, glm::vec3>& newValue) override;

	void EmitEvent(const std::pair<glm::vec3, glm::vec3>& oldValue, const std::pair<glm::vec3, glm::vec3>& newValue) override
	{
		_asset->EmitModelChanged(ModelBBoxChangeEvent{_id, newValue});
	}
};

class ChangeCBoxCommand : public ModelUndoCommand<std::pair<glm::vec3, glm::vec3>>
{
public:
	ChangeCBoxCommand(StudioModelAsset* asset, const std::pair<glm::vec3, glm::vec3>& oldEyePosition, const std::pair<glm::vec3, glm::vec3>& newEyePosition)
		: ModelUndoCommand(asset, ModelChangeId::ChangeCBox, oldEyePosition, newEyePosition)
	{
		setText("Change clipping box");
	}

protected:
	bool CanMerge(const ModelUndoCommand<std::pair<glm::vec3, glm::vec3>>* other) override
	{
		return _oldValue != other->GetNewValue();
	}

	void Apply(const std::pair<glm::vec3, glm::vec3>& oldValue, const std::pair<glm::vec3, glm::vec3>& newValue) override;

	void EmitEvent(const std::pair<glm::vec3, glm::vec3>& oldValue, const std::pair<glm::vec3, glm::vec3>& newValue) override
	{
		_asset->EmitModelChanged(ModelCBoxChangeEvent{_id, newValue});
	}
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
	bool CanMerge(const ModelListUndoCommand<QString>* other) override
	{
		return _oldValue != other->GetNewValue();
	}

	void Apply(int index, const QString& oldValue, const QString& newValue) override;
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
	bool CanMerge(const ModelListUndoCommand<int>* other) override
	{
		return _oldValue != other->GetNewValue();
	}

	void Apply(int index, const int& oldValue, const int& newValue) override;
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
	bool CanMerge(const ModelListUndoCommand<int>* other) override
	{
		return _oldValue != other->GetNewValue();
	}

	void Apply(int index, const int& oldValue, const int& newValue) override;
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
	void Apply(int index, const ChangeBoneProperties& oldValue, const ChangeBoneProperties& newValue) override;
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
	bool CanMerge(const ModelListUndoCommand<QString>* other) override
	{
		return _oldValue != other->GetNewValue();
	}

	void Apply(int index, const QString& oldValue, const QString& newValue) override;
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
	bool CanMerge(const ModelListUndoCommand<int>* other) override
	{
		return _oldValue != other->GetNewValue();
	}

	void Apply(int index, const int& oldValue, const int& newValue) override;
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
	bool CanMerge(const ModelListUndoCommand<int>* other) override
	{
		return _oldValue != other->GetNewValue();
	}

	void Apply(int index, const int& oldValue, const int& newValue) override;
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
	bool CanMerge(const ModelListUndoCommand<glm::vec3>* other) override
	{
		return _oldValue != other->GetNewValue();
	}

	void Apply(int index, const glm::vec3& oldValue, const glm::vec3& newValue) override;
};

class ChangeBoneControllerBoneCommand : public ModelListUndoCommand<int>
{
public:
	ChangeBoneControllerBoneCommand(StudioModelAsset* asset, int boneControllerIndex, int oldBone, int newBone)
		: ModelListUndoCommand(asset, ModelChangeId::ChangeBoneControllerBone, boneControllerIndex, oldBone, newBone)
	{
		setText("Change bone controller bone");
	}

protected:
	bool CanMerge(const ModelListUndoCommand<int>* other) override
	{
		return _oldValue != other->GetNewValue();
	}

	void Apply(int index, const int& oldValue, const int& newValue) override;
};

struct ChangeBoneControllerRange
{
	float Start;
	float End;

	bool operator!=(const ChangeBoneControllerRange& other) const
	{
		return Start != other.Start || End != other.End;
	}
};

class ChangeBoneControllerRangeCommand : public ModelListUndoCommand<ChangeBoneControllerRange>
{
public:
	ChangeBoneControllerRangeCommand(StudioModelAsset* asset, int boneControllerIndex,
		const ChangeBoneControllerRange& oldRange, const ChangeBoneControllerRange& newRange)
		: ModelListUndoCommand(asset, ModelChangeId::ChangeBoneControllerRange, boneControllerIndex, oldRange, newRange)
	{
		setText("Change bone controller range");
	}

protected:
	bool CanMerge(const ModelListUndoCommand<ChangeBoneControllerRange>* other) override
	{
		return _oldValue != other->GetNewValue();
	}

	void Apply(int index, const ChangeBoneControllerRange& oldValue, const ChangeBoneControllerRange& newValue) override;
};

class ChangeBoneControllerRestCommand : public ModelListUndoCommand<int>
{
public:
	ChangeBoneControllerRestCommand(StudioModelAsset* asset, int boneControllerIndex, int oldRest, int newRest)
		: ModelListUndoCommand(asset, ModelChangeId::ChangeBoneControllerRest, boneControllerIndex, oldRest, newRest)
	{
		setText("Change bone controller rest");
	}

protected:
	bool CanMerge(const ModelListUndoCommand<int>* other) override
	{
		return _oldValue != other->GetNewValue();
	}

	void Apply(int index, const int& oldValue, const int& newValue) override;
};

class ChangeBoneControllerIndexCommand : public ModelListUndoCommand<int>
{
public:
	ChangeBoneControllerIndexCommand(StudioModelAsset* asset, int boneControllerIndex, int oldIndex, int newIndex)
		: ModelListUndoCommand(asset, ModelChangeId::ChangeBoneControllerIndex, boneControllerIndex, oldIndex, newIndex)
	{
		setText("Change bone controller index");
	}

protected:
	bool CanMerge(const ModelListUndoCommand<int>* other) override
	{
		return _oldValue != other->GetNewValue();
	}

	void Apply(int index, const int& oldValue, const int& newValue) override;
};

class ChangeBoneControllerTypeCommand : public ModelListUndoCommand<int>
{
public:
	ChangeBoneControllerTypeCommand(StudioModelAsset* asset, int boneControllerIndex, int oldType, int newType)
		: ModelListUndoCommand(asset, ModelChangeId::ChangeBoneControllerType, boneControllerIndex, oldType, newType)
	{
		setText("Change bone controller type");
	}

protected:
	bool CanMerge(const ModelListUndoCommand<int>* other) override
	{
		return _oldValue != other->GetNewValue();
	}

	void Apply(int index, const int& oldValue, const int& newValue) override;
};

class ChangeModelFlagsCommand : public ModelUndoCommand<int>
{
public:
	ChangeModelFlagsCommand(StudioModelAsset* asset, int oldFlags, int newFlags)
		: ModelUndoCommand(asset, ModelChangeId::ChangeModelFlags, oldFlags, newFlags)
	{
		setText("Change model flags");
	}

protected:
	bool CanMerge(const ModelUndoCommand<int>* other) override
	{
		return _oldValue != other->GetNewValue();
	}

	void Apply(const int& oldValue, const int& newValue) override;
};

struct RootBoneData
{
	int BoneIndex;
	glm::vec3 BonePosition;
};

struct ChangeModelOriginData
{
	std::vector<RootBoneData> BoneData;
	glm::vec3 Offset;
};

class ChangeModelOriginCommand : public ModelUndoCommand<ChangeModelOriginData>
{
public:
	ChangeModelOriginCommand(StudioModelAsset* asset, ChangeModelOriginData&& oldPositions, ChangeModelOriginData&& newPositions)
		: ModelUndoCommand(asset, ModelChangeId::ChangeModelOrigin, std::move(oldPositions), std::move(newPositions))
	{
		setText("Change model origin");
	}

protected:
	void Apply(const ChangeModelOriginData& oldValue, const ChangeModelOriginData& newValue) override;

	void EmitEvent(const ChangeModelOriginData& oldValue, const ChangeModelOriginData& newValue) override
	{
		_asset->EmitModelChanged(ModelOriginChangeEvent(_id, newValue.Offset));
	}
};

class ChangeModelMeshesScaleCommand : public ModelUndoCommand<studiomdl::ScaleMeshesData>
{
public:
	ChangeModelMeshesScaleCommand(
		StudioModelAsset* asset, studiomdl::ScaleMeshesData&& oldData, studiomdl::ScaleMeshesData&& newData)
		: ModelUndoCommand(asset, ModelChangeId::ChangeModelMeshesScale, std::move(oldData), std::move(newData))
	{
		setText("Scale model meshes");
	}

protected:
	void Apply(const studiomdl::ScaleMeshesData& oldValue, const studiomdl::ScaleMeshesData& newValue) override;
};

class ChangeModelBonesScaleCommand : public ModelUndoCommand<std::vector<studiomdl::ScaleBonesBoneData>>
{
public:
	ChangeModelBonesScaleCommand(
		StudioModelAsset* asset, std::vector<studiomdl::ScaleBonesBoneData>&& oldData, std::vector<studiomdl::ScaleBonesBoneData>&& newData)
		: ModelUndoCommand(asset, ModelChangeId::ChangeModelBonesScale, std::move(oldData), std::move(newData))
	{
		setText("Scale model bones");
	}

protected:
	void Apply(const std::vector<studiomdl::ScaleBonesBoneData>& oldValue, const std::vector<studiomdl::ScaleBonesBoneData>& newValue) override;
};

class ChangeHitboxBoneCommand : public ModelListUndoCommand<int>
{
public:
	ChangeHitboxBoneCommand(StudioModelAsset* asset, int hitboxIndex, int oldBone, int newBone)
		: ModelListUndoCommand(asset, ModelChangeId::ChangeHitboxBone, hitboxIndex, oldBone, newBone)
	{
		setText("Change hitbox bone");
	}

protected:
	bool CanMerge(const ModelListUndoCommand<int>* other) override
	{
		return _oldValue != other->GetNewValue();
	}

	void Apply(int index, const int& oldValue, const int& newValue) override;
};

class ChangeHitboxHitgroupCommand : public ModelListUndoCommand<int>
{
public:
	ChangeHitboxHitgroupCommand(StudioModelAsset* asset, int hitboxIndex, int oldHitgroup, int newHitgroup)
		: ModelListUndoCommand(asset, ModelChangeId::ChangeHitboxHitgroup, hitboxIndex, oldHitgroup, newHitgroup)
	{
		setText("Change hitbox hitgroup");
	}

protected:
	bool CanMerge(const ModelListUndoCommand<int>* other) override
	{
		return _oldValue != other->GetNewValue();
	}

	void Apply(int index, const int& oldValue, const int& newValue) override;
};

class ChangeHitboxBoundsCommand : public ModelListUndoCommand<std::pair<glm::vec3, glm::vec3>>
{
public:
	ChangeHitboxBoundsCommand(StudioModelAsset* asset, int hitboxIndex,
		const std::pair<glm::vec3, glm::vec3>& oldBounds, const std::pair<glm::vec3, glm::vec3>& newBounds)
		: ModelListUndoCommand(asset, ModelChangeId::ChangeHitboxBounds, hitboxIndex, oldBounds, newBounds)
	{
		setText("Change hitbox bounds");
	}

protected:
	bool CanMerge(const ModelListUndoCommand<std::pair<glm::vec3, glm::vec3>>* other) override
	{
		return _oldValue != other->GetNewValue();
	}

	void Apply(int index, const std::pair<glm::vec3, glm::vec3>& oldValue, const std::pair<glm::vec3, glm::vec3>& newValue) override;
};

class ChangeTextureNameCommand : public ModelListUndoCommand<QString>
{
public:
	ChangeTextureNameCommand(StudioModelAsset* asset, int textureIndex, QString&& oldName, QString&& newName)
		: ModelListUndoCommand(asset, ModelChangeId::ChangeTextureName, textureIndex, std::move(oldName), std::move(newName))
	{
		setText("Change texture name");
	}

protected:
	bool CanMerge(const ModelListUndoCommand<QString>* other) override
	{
		return _oldValue != other->GetNewValue();
	}

	void Apply(int index, const QString& oldValue, const QString& newValue) override;
};

class ChangeTextureFlagsCommand : public ModelListUndoCommand<int>
{
public:
	ChangeTextureFlagsCommand(StudioModelAsset* asset, int textureIndex, int oldFlags, int newFlags)
		: ModelListUndoCommand(asset, ModelChangeId::ChangeTextureFlags, textureIndex, oldFlags, newFlags)
	{
		setText("Change texture flags");
	}

protected:
	bool CanMerge(const ModelListUndoCommand<int>* other) override
	{
		return _oldValue != other->GetNewValue();
	}

	void Apply(int index, const int& oldValue, const int& newValue) override;
};

struct ImportTextureData
{
	int Width{};
	int Height{};
	std::unique_ptr<byte[]> Pixels;
	byte Palette[PALETTE_SIZE]{};

	ImportTextureData() = default;

	ImportTextureData& operator=(const ImportTextureData& other)
	{
		if (this != &other)
		{
			Width = other.Width;
			Height = other.Height;

			Pixels = std::make_unique<byte[]>(Width * Height);

			memcpy(Pixels.get(), other.Pixels.get(), Width * Height);
			memcpy(Palette, other.Palette, sizeof(Palette));
		}

		return *this;
	}

	ImportTextureData(ImportTextureData&&) = default;
};

class ImportTextureCommand : public ModelListUndoCommand<ImportTextureData>
{
public:
	ImportTextureCommand(StudioModelAsset* asset, int textureIndex, ImportTextureData&& oldTexture, ImportTextureData&& newTexture)
		: ModelListUndoCommand(asset, ModelChangeId::ImportTexture, textureIndex, std::move(oldTexture), std::move(newTexture))
	{
		setText("Import texture");
	}

protected:
	void Apply(int index, const ImportTextureData& oldValue, const ImportTextureData& newValue) override;
};

class ChangeEventCommand : public ModelListUndoCommand<studiomdl::SequenceEvent>
{
public:
	ChangeEventCommand(StudioModelAsset* asset, int sequenceIndex, int eventIndex,
		const studiomdl::SequenceEvent& oldEvent, const studiomdl::SequenceEvent& newEvent)
		: ModelListUndoCommand(asset, ModelChangeId::ChangeEvent, sequenceIndex, oldEvent, newEvent)
		, _eventIndex(eventIndex)
	{
		setText("Change event");
	}

protected:
	void Apply(int index, const studiomdl::SequenceEvent& oldValue, const studiomdl::SequenceEvent& newValue) override;

	void EmitEvent(const studiomdl::SequenceEvent& oldValue, const studiomdl::SequenceEvent& newValue) override
	{
		_asset->EmitModelChanged(ModelEventChangeEvent(_id, _index, _eventIndex));
	}

private:
	const int _eventIndex;
};

class ChangeModelNameCommand : public ModelListUndoCommand<QString>
{
public:
	ChangeModelNameCommand(StudioModelAsset* asset, int bodyPartIndex, int modelIndex, QString&& oldName, QString&& newName)
		: ModelListUndoCommand(asset, ModelChangeId::ChangeModelName, bodyPartIndex, std::move(oldName), std::move(newName))
		, _modelIndex(modelIndex)
	{
		setText("Change model name");
	}

protected:
	bool CanMerge(const ModelListUndoCommand<QString>* other) override
	{
		return _oldValue != other->GetNewValue();
	}

	void Apply(int index, const QString& oldValue, const QString& newValue) override;

	void EmitEvent(const QString& oldValue, const QString& newValue) override
	{
		_asset->EmitModelChanged(ModelListSubListChangeEvent{_id, _index, -1, _modelIndex, -1});
	}

protected:
	const int _modelIndex;
};
}
