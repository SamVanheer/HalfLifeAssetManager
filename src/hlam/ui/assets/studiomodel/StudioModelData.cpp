#include "formats/studiomodel/EditableStudioModel.hpp"

#include "ui/assets/studiomodel/StudioModelData.hpp"
#include "ui/assets/studiomodel/StudioModelTextureUtilities.hpp"

namespace ui::assets::studiomodel
{
// TODO: need to use qt::GetEmptyModel to create an empty model
StudioModelData* StudioModelData::GetEmptyModel()
{
	static StudioModelData modelData{nullptr};
	return &modelData;
}

StudioModelData::StudioModelData(QObject* parent)
	: QObject(parent)
	, Attachments(new QStandardItemModel(this))
	, BodyParts(new QStandardItemModel(this))
	, Bones(new QStandardItemModel(this))
	, BonesWithNone(new QStandardItemModel(this))
	, BoneControllers(new QStandardItemModel(this))
	, BoneControllersWithNone(new QStandardItemModel(this))
	, Hitboxes(new QStandardItemModel(this))
	, Sequences(new QStandardItemModel(this))
	, Skins(new QStandardItemModel(this))
	, Textures(new QStandardItemModel(this))
{
}

void StudioModelData::Initialize(studiomdl::EditableStudioModel* model)
{
	const auto initializer = []<typename TCallback>(QStandardItemModel* itemModel, const auto& source, TCallback callback)
	{
		itemModel->clear();
		itemModel->setRowCount(source.size());

		for (int index = 0; const auto & element : source)
		{
			callback(itemModel, index, element);
			++index;
		}
	};

	initializer(Attachments, model->Attachments, [](auto itemModel, int index, const auto& element)
		{
			itemModel->setItem(index, new QStandardItem(QString{"Attachment %1"}.arg(index + 1)));
		});

	initializer(BodyParts, model->Bodyparts, [](auto itemModel, int index, const auto& element)
		{
			itemModel->setItem(index, new QStandardItem(QString::fromStdString(element->Name)));
		});

	Bones->clear();
	Bones->setRowCount(model->Bones.size());

	BonesWithNone->clear();
	BonesWithNone->setRowCount(model->Bones.size() + 1);

	BonesWithNone->setItem(0, new QStandardItem("None (-1)"));

	for (int index = 0; const auto& bone : model->Bones)
	{
		const QString text = QString{"%1 (%2)"}.arg(QString::fromStdString(bone->Name)).arg(index);
		Bones->setItem(index, new QStandardItem(text));
		BonesWithNone->setItem(index + 1, new QStandardItem(text));
		++index;
	}

	BoneControllers->clear();
	BoneControllers->setRowCount(model->BoneControllers.size());

	BoneControllersWithNone->clear();
	BoneControllersWithNone->setRowCount(model->BoneControllers.size() + 1);

	BoneControllersWithNone->setItem(0, new QStandardItem("None (-1)"));

	for (int index = 0; const auto& boneController : model->BoneControllers)
	{
		const QString label = boneController->Index == STUDIO_MOUTH_CONTROLLER ? "Mouth" : QString{"Controller %1"}.arg(boneController->Index);
		BoneControllers->setItem(index, new QStandardItem(label));
		BoneControllersWithNone->setItem(index + 1, new QStandardItem(label));
		++index;
	}

	initializer(Hitboxes, model->Hitboxes, [](auto itemModel, int index, const auto& element)
		{
			itemModel->setItem(index, new QStandardItem(QString{"Hitbox %1"}.arg(index + 1)));
		});

	initializer(Sequences, model->Sequences, [](auto itemModel, int index, const auto& element)
		{
			itemModel->setItem(index, new QStandardItem(QString::fromStdString(element->Label)));
		});

	initializer(Skins, model->SkinFamilies, [](auto itemModel, int index, const auto& element)
		{
			itemModel->setItem(index, new QStandardItem(QString{"Skin %1"}.arg(index + 1)));
		});

	initializer(Textures, model->Textures, [](auto itemModel, int index, const auto& element)
		{
			itemModel->setItem(index, new QStandardItem(FormatTextureName(*element)));
		});
}
}
