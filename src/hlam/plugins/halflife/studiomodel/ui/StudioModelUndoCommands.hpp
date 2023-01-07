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

	ChangeBoneProps,
	ChangeAttachmentProps,
	ChangeBoneControllerProps,

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

struct BoneProps
{
	std::string Name;
	int Flags{};
	std::array<glm::vec3, 2> Values;
	std::array<glm::vec3, 2> Scales;

	constexpr bool operator==(const BoneProps& other) const
	{
		if (Name != other.Name)
		{
			return false;
		}

		if (Flags != other.Flags)
		{
			return false;
		}

		for (std::size_t j = 0; j < Values.size(); ++j)
		{
			for (int i = 0; i < glm::vec3::length(); ++i)
			{
				if (Values[j][i] != other.Values[j][i] || Scales[j][i] != other.Scales[j][i])
				{
					return false;
				}
			}
		}

		return true;
	}
};

class ChangeBonePropsCommand : public ModelListUndoCommand<BoneProps>
{
public:
	ChangeBonePropsCommand(StudioModelAsset* asset, int boneIndex,
		const BoneProps& oldProps, const BoneProps& newProps)
		: ModelListUndoCommand(asset, ModelChangeId::ChangeBoneProps, boneIndex, oldProps, newProps)
	{
		setText("Change bone properties");
	}

protected:
	bool CanMerge(const ModelListUndoCommand<BoneProps>* other) override
	{
		return _oldValue != other->GetNewValue();
	}

	void Apply(int index, const BoneProps& oldValue, const BoneProps& newValue) override;
};

struct AttachmentProps
{
	std::string Name;
	int Type{};
	int Bone{};
	glm::vec3 Origin{0};

	constexpr auto operator<=>(const AttachmentProps&) const = default;
};

class ChangeAttachmentPropsCommand : public ModelListUndoCommand<AttachmentProps>
{
public:
	ChangeAttachmentPropsCommand(StudioModelAsset* asset, int attachmentIndex, const AttachmentProps& oldProps, const AttachmentProps& newProps)
		: ModelListUndoCommand(asset, ModelChangeId::ChangeAttachmentProps, attachmentIndex, oldProps, newProps)
	{
		setText("Change attachment properties");
	}

protected:
	bool CanMerge(const ModelListUndoCommand<AttachmentProps>* other) override
	{
		return _oldValue != other->GetNewValue();
	}

	void Apply(int index, const AttachmentProps& oldValue, const AttachmentProps& newValue) override;
};

struct BoneControllerProps
{
	float Start{};
	float End{};
	int Rest{};
	int Index{};
	int Bone{};
	int BoneAxis{};

	constexpr auto operator<=>(const BoneControllerProps&) const = default;
};

class ChangeBoneControllerPropsCommand : public ModelListUndoCommand<BoneControllerProps>
{
public:
	ChangeBoneControllerPropsCommand(StudioModelAsset* asset, int boneControllerIndex,
		const BoneControllerProps& oldProps, const BoneControllerProps& newProps)
		: ModelListUndoCommand(asset, ModelChangeId::ChangeBoneControllerProps, boneControllerIndex, oldProps, newProps)
	{
		setText("Change bone controller properties");
	}

protected:
	bool CanMerge(const ModelListUndoCommand<BoneControllerProps>* other) override
	{
		return _oldValue != other->GetNewValue();
	}

	void Apply(int index, const BoneControllerProps& oldValue, const BoneControllerProps& newValue) override;
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

class ChangeModelOriginCommand : public BaseModelUndoCommand
{
public:
	ChangeModelOriginCommand(StudioModelAsset* asset,
		std::vector<studiomdl::MoveBoneData>&& data, const glm::vec3& offset)
		: BaseModelUndoCommand(asset, ModelChangeId::ChangeModelOrigin)
		, _data(std::move(data))
		, _offset(offset)
	{
		setText("Change model origin");
	}

	void undo() override;
	void redo() override;

private:
	const std::vector<studiomdl::MoveBoneData> _data;
	const glm::vec3 _offset;
};

class ChangeModelScaleCommand : public BaseModelUndoCommand
{
public:
	ChangeModelScaleCommand(StudioModelAsset* asset, studiomdl::ScaleData&& data, float scale)
		: BaseModelUndoCommand(asset, ModelChangeId::ChangeModelScale)
		, _data(std::move(data))
		, _scale(scale)
	{
		setText("Scale model");
	}

	void undo() override;
	void redo() override;

private:
	const studiomdl::ScaleData _data;
	const float _scale;
};

class ChangeModelRotationCommand : public BaseModelUndoCommand
{
public:
	ChangeModelRotationCommand(
		StudioModelAsset* asset, std::vector<studiomdl::RotateBoneData>&& data, const glm::vec3& angles)
		: BaseModelUndoCommand(asset, ModelChangeId::ChangeModelRotation)
		, _data(std::move(data))
		, _angles(angles)
	{
		setText("Rotate model");
	}

	void undo() override;
	void redo() override;

private:
	const std::vector<studiomdl::RotateBoneData> _data;
	const glm::vec3 _angles;
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
