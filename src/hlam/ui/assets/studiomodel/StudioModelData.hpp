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
	Q_OBJECT

private:
	explicit StudioModelData(QObject* parent);

public:
	static StudioModelData* GetEmptyModel();

	StudioModelData(studiomdl::EditableStudioModel* model, QObject* parent);

	void DisconnectFromAll(QObject* receiver);

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

signals:
	void ModelOriginChanged();
	void ModelScaleChanged();
	void ModelRotationChanged();

	void EyePositionChanged();
	void ModelBBoxChanged();
	void ModelCBoxChanged();
	void ModelFlagsChanged();

	void AttachmentDataChanged(int index);

	void BoneDataChanged(int index);

	void BoneControllerDataChanged(int index);

	void SubModelNameChanged(int bodyPartIndex, int modelIndex);

	void HitboxDataChanged(int index);

	void EventChanged(int sequenceIndex, int eventIndex);
	void EventAdded(int sequenceIndex, int eventIndex);
	void EventRemoved(int sequenceIndex, int eventIndex);

	void TextureNameChanged(int index);
	void TextureFlagsChanged(int index);
};
}
