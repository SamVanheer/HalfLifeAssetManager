#pragma once

#include <QObject>

class QAbstractItemModel;

namespace studiomdl
{
class EditableStudioModel;
}

namespace ui::assets::studiomodel
{
class StudioModelData final : public QObject
{
private:
	explicit StudioModelData(QObject* parent);

public:
	static StudioModelData* GetEmptyModel();

	StudioModelData(studiomdl::EditableStudioModel* model, QObject* parent);

	QAbstractItemModel* const Attachments;
	QAbstractItemModel* const BodyParts;
	QAbstractItemModel* const Bones;
	QAbstractItemModel* const BonesWithNone;
	QAbstractItemModel* const BoneControllers;
	QAbstractItemModel* const BoneControllersWithNone;
	QAbstractItemModel* const Hitboxes;
	QAbstractItemModel* const Sequences;
	QAbstractItemModel* const Skins;
	QAbstractItemModel* const Textures;
};
}
