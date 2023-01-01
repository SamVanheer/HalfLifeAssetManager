#include <QAbstractItemModel>

#include "ui_ModelInfoDialog.h"

#include "plugins/halflife/studiomodel/StudioModelAsset.hpp"
#include "plugins/halflife/studiomodel/ui/StudioModelData.hpp"
#include "plugins/halflife/studiomodel/ui/dialogs/ModelInfoDialog.hpp"

namespace studiomodel
{
ModelInfoDialog::ModelInfoDialog(StudioModelAssetProvider* provider, QWidget* parent)
	: QDialog(parent)
	, _ui(std::make_unique<Ui_ModelInfoDialog>())
	, _provider(provider)
	, _asset(_provider->GetCurrentAsset())
{
	_ui->setupUi(this);

	const auto model = _asset->GetEditableStudioModel();
	const auto modelData = _asset->GetModelData();

	_ui->BonesValue->setText(QString::number(modelData->Bones->rowCount()));
	_ui->BoneControllersValue->setText(QString::number(modelData->BoneControllers->rowCount()));
	_ui->HitBoxesValue->setText(QString::number(modelData->Hitboxes->rowCount()));
	_ui->SequencesValue->setText(QString::number(modelData->Sequences->rowCount()));
	_ui->SequenceGroupsValue->setText(QString::number(model->SequenceGroups.size()));

	_ui->TexturesValue->setText(QString::number(modelData->Textures->rowCount()));
	_ui->SkinFamiliesValue->setText(QString::number(modelData->Skins->rowCount()));
	_ui->BodyPartsValue->setText(QString::number(modelData->BodyParts->rowCount()));
	_ui->AttachmentsValue->setText(QString::number(modelData->Attachments->rowCount()));
	_ui->TransitionsValue->setText(QString::number(model->Transitions.size()));
}

ModelInfoDialog::~ModelInfoDialog() = default;
}
