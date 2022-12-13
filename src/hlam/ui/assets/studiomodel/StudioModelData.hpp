#pragma once

#include <QObject>
#include <QStandardItemModel>

namespace studiomdl
{
class EditableStudioModel;
}

namespace ui::assets::studiomodel
{
class StudioModelData final : public QObject
{
public:
	static StudioModelData* GetEmptyModel();

	StudioModelData(QObject* parent);

	void Initialize(studiomdl::EditableStudioModel* model);

	QStandardItemModel* const Attachments;
	QStandardItemModel* const BodyParts;
	QStandardItemModel* const Bones;
	QStandardItemModel* const BonesWithNone;
	QStandardItemModel* const BoneControllers;
	QStandardItemModel* const BoneControllersWithNone;
	QStandardItemModel* const Hitboxes;
	QStandardItemModel* const Sequences;
	QStandardItemModel* const Skins;
	QStandardItemModel* const Textures;
};
}
