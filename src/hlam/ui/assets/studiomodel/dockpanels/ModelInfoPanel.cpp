#include <QSignalBlocker>

#include "ui/assets/studiomodel/StudioModelAsset.hpp"
#include "ui/assets/studiomodel/StudioModelData.hpp"
#include "ui/assets/studiomodel/dockpanels/ModelInfoPanel.hpp"

namespace ui::assets::studiomodel
{
ModelInfoPanel::ModelInfoPanel(StudioModelAsset* asset)
	: _asset(asset)
{
	_ui.setupUi(this);

	connect(_asset, &StudioModelAsset::AssetChanged, this, &ModelInfoPanel::OnAssetChanged);

	OnAssetChanged(nullptr);

	//TODO: listen to changes made to the model to update values
}

ModelInfoPanel::~ModelInfoPanel() = default;

void ModelInfoPanel::OnAssetChanged(StudioModelAsset* asset)
{
	auto modelData = asset ? asset->GetModelData() : StudioModelData::GetEmptyModel();

	// TODO: this panel isn't terribly useful. Folding this information into the other panels will largely eliminate the need for this.
	// Either way, this panel should be turned into a dialog to show the extra info on-demand, since it's just cluttering up the UI.

	//TODO
	/*
	_ui.BonesValue->setText(QString::number(modelData->Bones->rowCount()));
	_ui.BoneControllersValue->setText(QString::number(modelData->BoneControllers->rowCount()));
	_ui.HitBoxesValue->setText(QString::number(modelData->Hitboxes->rowCount()));
	_ui.SequencesValue->setText(QString::number(modelData->Sequences->rowCount()));
	_ui.SequenceGroupsValue->setText(QString::number(modelData->SequenceGroups->rowCount()));

	_ui.TexturesValue->setText(QString::number(modelData->Textures->rowCount()));
	_ui.SkinFamiliesValue->setText(QString::number(modelData->Skins->rowCount()));
	_ui.BodyPartsValue->setText(QString::number(modelData->BodyParts->rowCount()));
	_ui.AttachmentsValue->setText(QString::number(modelData->Attachments->rowCount()));
	_ui.TransitionsValue->setText(QString::number(modelData->Transitions->rowCount()));
	*/
}
}
