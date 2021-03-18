#include <QSignalBlocker>

#include "entity/HLMVStudioModelEntity.hpp"

#include "ui/assets/studiomodel/StudioModelAsset.hpp"
#include "ui/assets/studiomodel/dockpanels/StudioModelModelInfoPanel.hpp"

namespace ui::assets::studiomodel
{
StudioModelModelInfoPanel::StudioModelModelInfoPanel(StudioModelAsset* asset, QWidget* parent)
	: QWidget(parent)
	, _asset(asset)
{
	_ui.setupUi(this);

	auto entity = _asset->GetScene()->GetEntity();
	auto model = entity->GetEditableModel();

	_ui.BonesValue->setText(QString::number(model->Bones.size()));
	_ui.BoneControllersValue->setText(QString::number(model->BoneControllers.size()));
	_ui.HitBoxesValue->setText(QString::number(model->Hitboxes.size()));
	_ui.SequencesValue->setText(QString::number(model->Sequences.size()));
	_ui.SequenceGroupsValue->setText(QString::number(model->SequenceGroups.size()));

	_ui.TexturesValue->setText(QString::number(model->Textures.size()));
	_ui.SkinFamiliesValue->setText(QString::number(model->SkinFamilies.size()));
	_ui.BodyPartsValue->setText(QString::number(model->Bodyparts.size()));
	_ui.AttachmentsValue->setText(QString::number(model->Attachments.size()));
	_ui.TransitionsValue->setText(QString::number(model->Transitions.size()));

	//TODO: listen to changes made to the model to update values
}

StudioModelModelInfoPanel::~StudioModelModelInfoPanel() = default;
}
