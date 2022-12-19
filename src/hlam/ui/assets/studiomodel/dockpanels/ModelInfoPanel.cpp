#include <QAbstractItemModel>

#include "ui/assets/studiomodel/StudioModelAsset.hpp"
#include "ui/assets/studiomodel/StudioModelData.hpp"
#include "ui/assets/studiomodel/dockpanels/ModelInfoPanel.hpp"

namespace studiomodel
{
ModelInfoPanel::ModelInfoPanel(StudioModelAsset* asset)
	: _asset(asset)
{
	_ui.setupUi(this);

	connect(_asset, &StudioModelAsset::AssetChanged, this, &ModelInfoPanel::OnAssetChanged);

	OnAssetChanged(_asset->GetProvider()->GetDummyAsset());
}

ModelInfoPanel::~ModelInfoPanel() = default;

void ModelInfoPanel::OnAssetChanged(StudioModelAsset* asset)
{
	const auto model = asset->GetEditableStudioModel();
	auto modelData = asset->GetModelData();

	if (_previousModelData)
	{
		_previousModelData->DisconnectFromAll(this);
	}

	_previousModelData = modelData;

	const auto itemModelSetup = [this](QLabel* label, QAbstractItemModel* itemModel)
	{
		const auto lambda = [label, itemModel]()
		{
			label->setText(QString::number(itemModel->rowCount()));
		};
		
		// Immediately set the label.
		lambda();

		QObject::connect(itemModel, &QAbstractItemModel::rowsInserted, this, lambda);
		QObject::connect(itemModel, &QAbstractItemModel::rowsRemoved, this, lambda);
	};

	itemModelSetup(_ui.BonesValue, modelData->Bones);
	itemModelSetup(_ui.BoneControllersValue, modelData->BoneControllers);
	itemModelSetup(_ui.HitBoxesValue, modelData->Hitboxes);
	itemModelSetup(_ui.SequencesValue, modelData->Sequences);
	_ui.SequenceGroupsValue->setText(QString::number(model ? model->SequenceGroups.size() : 0));

	itemModelSetup(_ui.TexturesValue, modelData->Textures);
	itemModelSetup(_ui.SkinFamiliesValue, modelData->Skins);
	itemModelSetup(_ui.BodyPartsValue, modelData->BodyParts);
	itemModelSetup(_ui.AttachmentsValue, modelData->Attachments);
	_ui.TransitionsValue->setText(QString::number(model ? model->Transitions.size() : 0));

	// TODO: this panel isn't terribly useful. Folding this information into the other panels will largely eliminate the need for this.
	// Either way, this panel should be turned into a dialog to show the extra info on-demand, since it's just cluttering up the UI.
}
}
