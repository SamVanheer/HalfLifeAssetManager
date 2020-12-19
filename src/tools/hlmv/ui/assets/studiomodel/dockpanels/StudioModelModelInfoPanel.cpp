#include <limits>

#include <QSignalBlocker>

#include "entity/CHLMVStudioModelEntity.hpp"

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
	auto model = entity->GetModel()->GetStudioHeader();
	auto textureHeader = entity->GetModel()->GetTextureHeader();

	_ui.BonesValue->setText(QString::number(model->numbones));
	_ui.BoneControllersValue->setText(QString::number(model->numbonecontrollers));
	_ui.HitBoxesValue->setText(QString::number(model->numhitboxes));
	_ui.SequencesValue->setText(QString::number(model->numseq));
	_ui.SequenceGroupsValue->setText(QString::number(model->numseqgroups));

	_ui.TexturesValue->setText(QString::number(textureHeader->numtextures));
	_ui.SkinFamiliesValue->setText(QString::number(textureHeader->numskinfamilies));
	_ui.BodyPartsValue->setText(QString::number(model->numbodyparts));
	_ui.AttachmentsValue->setText(QString::number(model->numattachments));
	_ui.TransitionsValue->setText(QString::number(model->numtransitions));

	//TODO: listen to changes made to the model to update values
}

StudioModelModelInfoPanel::~StudioModelModelInfoPanel() = default;
}
