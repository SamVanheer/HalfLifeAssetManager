#include <memory>
#include <vector>

#include <QAbstractItemModel>

#include "formats/studiomodel/EditableStudioModel.hpp"

#include "plugins/halflife/studiomodel/ui/StudioModelData.hpp"
#include "plugins/halflife/studiomodel/ui/StudioModelTextureUtilities.hpp"

namespace studiomodel
{
/**
*	@brief Base class for item models for studiomodel data.
*/
template<typename TElement>
class BaseStudioModelItemModel : public QAbstractItemModel
{
protected:
	BaseStudioModelItemModel(std::vector<TElement>* items, QObject* parent, bool hasInvalidElement)
		: QAbstractItemModel(parent)
		, _items(items)
		, _hasInvalidElement(hasInvalidElement)
	{
	}

public:
	QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override
	{
		if (!_items || parent.isValid() || row < 0 || row >= rowCount() || column != 0)
		{
			return QModelIndex();
		}

		return createIndex(row, 0, nullptr);
	}

	QModelIndex parent(const QModelIndex& child) const override { return QModelIndex(); }

	int rowCount(const QModelIndex& parent = QModelIndex()) const override
	{
		if (!_items)
		{
			return 0;
		}

		int size = _items->size();

		if (_hasInvalidElement)
		{
			++size;
		}

		return size;
	}

	int columnCount(const QModelIndex& parent = QModelIndex()) const override { return 1; }
	bool hasChildren(const QModelIndex& parent = QModelIndex()) const override { return false; }

	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override
	{
		if (_items && index.isValid() && index.row() < rowCount())
		{
			if (role == Qt::DisplayRole)
			{
				if (_hasInvalidElement && index.row() == 0)
				{
					return QVariant::fromValue(QString{"None (-1)"});
				}

				// Subclasses need not worry about the invalid element.
				QModelIndex adjustedIndex = index;

				if (_hasInvalidElement)
				{
					adjustedIndex = createIndex(index.row() - 1, 0, nullptr);
				}

				return QVariant::fromValue(ElementToString(adjustedIndex));
			}
		}

		return QVariant();
	}

protected:
	virtual QString ElementToString(const QModelIndex& index) const = 0;

protected:
	std::vector<TElement>* _items{};
	const bool _hasInvalidElement;
};

class AttachmentsItemModel final : public BaseStudioModelItemModel<std::unique_ptr<studiomdl::StudioAttachment>>
{
public:
	AttachmentsItemModel(std::vector<std::unique_ptr<studiomdl::StudioAttachment>>* items, QObject* parent)
		: BaseStudioModelItemModel(items, parent, false)
	{
	}

protected:
	QString ElementToString(const QModelIndex& index) const override
	{
		return QString{"Attachment %1"}.arg(index.row() + 1);
	}
};

class BodyPartsItemModel final : public BaseStudioModelItemModel<std::unique_ptr<studiomdl::StudioBodypart>>
{
public:
	BodyPartsItemModel(std::vector<std::unique_ptr<studiomdl::StudioBodypart>>* items, QObject* parent)
		: BaseStudioModelItemModel(items, parent, false)
	{
	}

protected:
	QString ElementToString(const QModelIndex& index) const override
	{
		return QString::fromStdString((*_items)[index.row()]->Name);
	}
};

class BonesItemModel final : public BaseStudioModelItemModel<std::unique_ptr<studiomdl::StudioBone>>
{
public:
	BonesItemModel(std::vector<std::unique_ptr<studiomdl::StudioBone>>* items, QObject* parent, bool hasInvalidElement)
		: BaseStudioModelItemModel(items, parent, hasInvalidElement)
	{
	}

protected:
	QString ElementToString(const QModelIndex& index) const override
	{
		return QString{"%1 (%2)"}.arg(QString::fromStdString((*_items)[index.row()]->Name)).arg(index.row());
	}
};

class BoneControllersItemModel final : public BaseStudioModelItemModel<std::unique_ptr<studiomdl::StudioBoneController>>
{
public:
	BoneControllersItemModel(std::vector<std::unique_ptr<studiomdl::StudioBoneController>>* items, QObject* parent)
		: BaseStudioModelItemModel(items, parent, false)
	{
	}

protected:
	QString ElementToString(const QModelIndex& index) const override
	{
		const auto& boneController = *(*_items)[index.row()];

		if (boneController.Index == STUDIO_MOUTH_CONTROLLER)
		{
			return "Mouth";
		}

		return QString{"Controller %1"}.arg(boneController.Index);
	}
};

class HitboxesItemModel final : public BaseStudioModelItemModel<std::unique_ptr<studiomdl::StudioHitbox>>
{
public:
	HitboxesItemModel(std::vector<std::unique_ptr<studiomdl::StudioHitbox>>* items, QObject* parent)
		: BaseStudioModelItemModel(items, parent, false)
	{
	}

protected:
	QString ElementToString(const QModelIndex& index) const override
	{
		return QString{"Hitbox %1"}.arg(index.row() + 1);
	}
};

class SequencesItemModel final : public BaseStudioModelItemModel<std::unique_ptr<studiomdl::StudioSequence>>
{
public:
	SequencesItemModel(std::vector<std::unique_ptr<studiomdl::StudioSequence>>* items, QObject* parent)
		: BaseStudioModelItemModel(items, parent, false)
	{
	}

protected:
	QString ElementToString(const QModelIndex& index) const override
	{
		// Trim excess whitespace and replace newlines with whitespace so names don't spread across multiple lines.
		QString sequenceName = QString::fromStdString((*_items)[index.row()]->Label)
			.trimmed()
			.replace('\r', ' ')
			.replace('\n', ' ');

		return QString{"%1 (%2)"}
			.arg(sequenceName)
			.arg(index.row());
	}
};

class SkinsItemModel final : public BaseStudioModelItemModel<std::vector<studiomdl::StudioTexture*>>
{
public:
	SkinsItemModel(std::vector<std::vector<studiomdl::StudioTexture*>>* items, QObject* parent)
		: BaseStudioModelItemModel(items, parent, false)
	{
	}

protected:
	QString ElementToString(const QModelIndex& index) const override
	{
		return QString{"Skin %1"}.arg(index.row() + 1);
	}
};

class TexturesItemModel final : public BaseStudioModelItemModel<std::unique_ptr<studiomdl::StudioTexture>>
{
public:
	TexturesItemModel(std::vector<std::unique_ptr<studiomdl::StudioTexture>>* items, QObject* parent)
		: BaseStudioModelItemModel(items, parent, false)
	{
	}

protected:
	QString ElementToString(const QModelIndex& index) const override
	{
		const auto& texture = *(*_items)[index.row()];

		return QString{"%1 (%2 x %3)"}
			.arg(QString::fromStdString(texture.Name))
			.arg(texture.Data.Width)
			.arg(texture.Data.Height);
	}
};

StudioModelData::StudioModelData(studiomdl::EditableStudioModel* model, QObject* parent)
	: QObject(parent)
	, Attachments(new AttachmentsItemModel(model ? &model->Attachments : nullptr, this))
	, BodyParts(new BodyPartsItemModel(model ? &model->Bodyparts : nullptr, this))
	, Bones(new BonesItemModel(model ? &model->Bones : nullptr, this, false))
	, BonesWithNone(new BonesItemModel(model ? &model->Bones : nullptr, this, true))
	, BoneControllers(new BoneControllersItemModel(model ? &model->BoneControllers : nullptr, this))
	, Hitboxes(new HitboxesItemModel(model ? &model->Hitboxes : nullptr, this))
	, Sequences(new SequencesItemModel(model ? &model->Sequences : nullptr, this))
	, Skins(new SkinsItemModel(model ? &model->SkinFamilies : nullptr, this))
	, Textures(new TexturesItemModel(model ? &model->Textures : nullptr, this))
{
}

void StudioModelData::DisconnectFromAll(QObject* receiver)
{
	this->disconnect(receiver);

	for (auto model :
		{
			Attachments,
			BodyParts,
			Bones,
			BonesWithNone,
			BoneControllers,
			Hitboxes,
			Sequences,
			Skins,
			Textures
		})
	{
		model->disconnect(receiver);
	}
}
}
