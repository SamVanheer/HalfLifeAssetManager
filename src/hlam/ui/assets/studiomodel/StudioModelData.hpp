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

	void AttachmentNameChanged(int index);
	void AttachmentTypeChanged(int index);
	void AttachmentBoneChanged(int index);
	void AttachmentOriginChanged(int index);

	void BoneParentChanged(int index);
	void BoneFlagsChanged(int index);
	void BonePropertiesChanged(int index);

	void BoneControllerRangeChanged(int index);
	void BoneControllerRestChanged(int index);
	void BoneControllerIndexChanged(int index);

	void BoneControllerChangedFromBone(int controllerIndex, int boneIndex);
	void BoneControllerChangedFromController(int controllerIndex, int boneIndex);

	void SubModelNameChanged(int bodyPartIndex, int modelIndex);

	void HitboxBoneChanged(int index);
	void HitboxHitgroupChanged(int index);
	void HitboxBoundsChanged(int index);

	void EventChanged(int sequenceIndex, int eventIndex);
	void EventAdded(int sequenceIndex, int eventIndex);
	void EventRemoved(int sequenceIndex, int eventIndex);

	void TextureNameChanged(int index);
	void TextureFlagsChanged(int index);
};
}
