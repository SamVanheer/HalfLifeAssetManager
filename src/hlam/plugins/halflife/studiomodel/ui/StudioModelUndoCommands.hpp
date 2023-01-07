#pragma once

#include <array>
#include <cassert>
#include <cstring>
#include <memory>
#include <vector>

#include <QString>
#include <QUndoStack>

#include <glm/vec3.hpp>

#include "formats/studiomodel/StudioModelFileFormat.hpp"
#include "graphics/Palette.hpp"

#include "plugins/halflife/studiomodel/StudioModelAsset.hpp"

class QAbstractItemModel;

namespace studiomdl
{
struct ScaleBonesBoneData;
struct ScaleMeshesData;
}

namespace studiomodel
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
	ChangeBoneControllerFromBone,

	ChangeAttachmentName,
	ChangeAttachmentType,
	ChangeAttachmentBone,
	ChangeAttachmentOrigin,

	ChangeBoneControllerRange,
	ChangeBoneControllerRest,
	ChangeBoneControllerIndex,
	ChangeBoneControllerFromController,

	ChangeModelFlags,
	ChangeModelOrigin,
	ChangeModelScale,
	ChangeModelRotation,

	ChangeHitboxProps,

	ChangeTextureName,
	ChangeTextureFlags,
	ImportTexture,

	ChangeSequenceProperties,

	ChangeEvent,
	AddRemoveEvent,

	ChangeModelName,

	FlipNormals,
};

enum class AddRemoveType
{
	Addition = 0,
	Removal
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
	void EmitDataChanged(QAbstractItemModel* model, int index);

protected:
	StudioModelAsset* const _asset;
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
	}

	void redo() override
	{
		Apply(_oldValue, _newValue);
	}

	const T& GetOldValue() const { return _oldValue; }

	const T& GetNewValue() const { return _newValue; }

protected:
	virtual bool CanMerge(const ModelUndoCommand* other) { return false; }

	virtual void Apply(const T& oldValue, const T& newValue) = 0;

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

		auto otherUndo = const_cast<ModelListUndoCommand*>(static_cast<const ModelListUndoCommand*>(other));

		//Didn't change the same list element
		if (_index != otherUndo->_index)
		{
			return false;
		}

		if (!CanMerge(otherUndo))
		{
			return false;
		}

		_newValue = std::move(otherUndo->_newValue);

		return true;
	}

	void undo() override
	{
		Apply(_index, _newValue, _oldValue);
	}

	void redo() override
	{
		Apply(_index, _oldValue, _newValue);
	}

	int GetIndex() const { return _index; }

	const T& GetOldValue() const { return _oldValue; }

	const T& GetNewValue() const { return _newValue; }

protected:
	virtual bool CanMerge(const ModelListUndoCommand* other) { return false; }

	virtual void Apply(int index, const T& oldValue, const T& newValue) = 0;

protected:
	const int _index;
	const T _oldValue;
	T _newValue;
};

/**
*	@brief Base class for addition and removal of entries in lists
*/
template<typename T>
class ModelListAddRemoveUndoCommand : public BaseModelUndoCommand
{
protected:
	ModelListAddRemoveUndoCommand(StudioModelAsset* asset, ModelChangeId id, AddRemoveType type, int index, const T& value)
		: BaseModelUndoCommand(asset, id)
		, _type(type)
		, _index(index)
		, _value(value)
	{
	}

public:
	void undo() override
	{
		Apply(false);
	}

	void redo() override
	{
		Apply(true);
	}

	AddRemoveType GetType() const { return _type; }

	int GetIndex() const { return _index; }

	const T& GetValue() const { return _value; }

protected:
	virtual void Add(int index, const T& value) = 0;
	virtual void Remove(int index, const T& value) = 0;

private:
	void Apply(bool redo)
	{
		AddRemoveType type = _type;

		//Swap type when undoing
		if (!redo)
		{
			switch (type)
			{
			case AddRemoveType::Addition:
				type = AddRemoveType::Removal;
				break;

			case AddRemoveType::Removal:
				type = AddRemoveType::Addition;
				break;
			}
		}

		if (type == AddRemoveType::Addition)
		{
			Add(_index, _value);
		}
		else
		{
			Remove(_index, _value);
		}
	}

protected:
	const AddRemoveType _type;
	const int _index;
	const T _value;
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

/**
*	@brief bone, axis
*/
using ChangeBoneControllerData = std::pair<int, int>;

class ChangeBoneControllerFromControllerCommand : public ModelListUndoCommand<ChangeBoneControllerData>
{
public:
	ChangeBoneControllerFromControllerCommand(StudioModelAsset* asset, int boneControllerIndex,
		const ChangeBoneControllerData& oldValue, const ChangeBoneControllerData& newValue)
		: ModelListUndoCommand(asset, ModelChangeId::ChangeBoneControllerFromController, boneControllerIndex, oldValue, newValue)
	{
		setText("Change bone controller");
	}

protected:
	bool CanMerge(const ModelListUndoCommand<ChangeBoneControllerData>* other) override
	{
		return _oldValue != other->GetNewValue();
	}

	void Apply(int index, const ChangeBoneControllerData& oldValue, const ChangeBoneControllerData& newValue) override;
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

class ChangeModelOriginCommand : public ModelUndoCommand<studiomdl::MoveData>
{
public:
	ChangeModelOriginCommand(StudioModelAsset* asset, studiomdl::MoveData&& oldPositions, studiomdl::MoveData&& newPositions)
		: ModelUndoCommand(asset, ModelChangeId::ChangeModelOrigin, std::move(oldPositions), std::move(newPositions))
	{
		setText("Change model origin");
	}

protected:
	void Apply(const studiomdl::MoveData& oldValue, const studiomdl::MoveData& newValue) override;
};

class ChangeModelScaleCommand : public ModelUndoCommand<studiomdl::ScaleData>
{
public:
	ChangeModelScaleCommand(
		StudioModelAsset* asset, studiomdl::ScaleData&& oldData, studiomdl::ScaleData&& newData)
		: ModelUndoCommand(asset, ModelChangeId::ChangeModelScale, std::move(oldData), std::move(newData))
	{
		setText("Scale model");
	}

protected:
	void Apply(const studiomdl::ScaleData& oldValue, const studiomdl::ScaleData& newValue) override;
};

class ChangeModelRotationCommand : public ModelUndoCommand<studiomdl::RotateData>
{
public:
	ChangeModelRotationCommand(
		StudioModelAsset* asset, studiomdl::RotateData&& oldData, studiomdl::RotateData&& newData)
		: ModelUndoCommand(asset, ModelChangeId::ChangeModelRotation, std::move(oldData), std::move(newData))
	{
		setText("Rotate model");
	}

protected:
	void Apply(const studiomdl::RotateData& oldValue, const studiomdl::RotateData& newValue) override;
};

struct HitboxProps
{
	int Bone{};
	int Group{};
	glm::vec3 Min{0};
	glm::vec3 Max{0};

	constexpr auto operator<=>(const HitboxProps&) const = default;
};

class ChangeHitboxPropsCommand : public ModelListUndoCommand<HitboxProps>
{
public:
	ChangeHitboxPropsCommand(StudioModelAsset* asset, int hitboxIndex,
		const HitboxProps& oldProps, const HitboxProps& newProps)
		: ModelListUndoCommand(asset, ModelChangeId::ChangeHitboxProps, hitboxIndex, oldProps, newProps)
	{
		setText("Change hitbox properties");
	}

protected:
	bool CanMerge(const ModelListUndoCommand* other) override
	{
		return _oldValue != other->GetNewValue();
	}

	void Apply(int index, const HitboxProps& oldValue, const HitboxProps& newValue) override;
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
	studiomdl::StudioTextureData Data;

	studiomdl::ScaleSTCoordinatesData ScaledSTCoordinates;

	ImportTextureData() = default;
	ImportTextureData(ImportTextureData&&) = default;
	ImportTextureData& operator=(ImportTextureData&& other) = default;
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

struct SequenceProps
{
	bool IsLooping{};
	float FPS{};
	int Activity{};
	int ActivityWeight{};
	glm::vec3 LinearMovement{0};
};

class ChangeSequencePropsCommand : public ModelListUndoCommand<SequenceProps>
{
public:
	ChangeSequencePropsCommand(StudioModelAsset* asset, int sequenceIndex, const SequenceProps& oldProps, const SequenceProps& newProps)
		: ModelListUndoCommand(
			asset, ModelChangeId::ChangeSequenceProperties, sequenceIndex, oldProps, newProps)
	{
		setText("Change sequence properties");
	}

protected:
	void Apply(int index, const SequenceProps& oldValue, const SequenceProps& newValue) override;
};

class ChangeEventCommand : public ModelListUndoCommand<studiomdl::StudioSequenceEvent>
{
public:
	ChangeEventCommand(StudioModelAsset* asset, int sequenceIndex, int eventIndex,
		const studiomdl::StudioSequenceEvent& oldEvent, const studiomdl::StudioSequenceEvent& newEvent)
		: ModelListUndoCommand(asset, ModelChangeId::ChangeEvent, sequenceIndex, oldEvent, newEvent)
		, _eventIndex(eventIndex)
	{
		setText("Change event");
	}

protected:
	void Apply(int index,
		const studiomdl::StudioSequenceEvent& oldValue, const studiomdl::StudioSequenceEvent& newValue) override;

private:
	const int _eventIndex;
};

class AddRemoveEventCommand : public ModelListAddRemoveUndoCommand<studiomdl::StudioSequenceEvent>
{
public:
	AddRemoveEventCommand(StudioModelAsset* asset, AddRemoveType type, int index, int eventIndex,
		const studiomdl::StudioSequenceEvent& value)
		: ModelListAddRemoveUndoCommand(asset, ModelChangeId::AddRemoveEvent, type, index, value)
		, _eventIndex(eventIndex)
	{
		if (type == AddRemoveType::Addition)
		{
			setText("Add event");
		}
		else
		{
			setText("Remove event");
		}
	}

protected:
	void Add(int index, const studiomdl::StudioSequenceEvent& value) override;
	void Remove(int index, const studiomdl::StudioSequenceEvent& value) override;

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

protected:
	const int _modelIndex;
};

class FlipNormalsCommand : public BaseModelUndoCommand
{
public:
	explicit FlipNormalsCommand(StudioModelAsset* asset);

	void undo() override;
	void redo() override;

private:
	std::vector<glm::vec3> _normals;
};
}
