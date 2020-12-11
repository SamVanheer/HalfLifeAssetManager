#include <limits>

#include <QSignalBlocker>

#include "entity/CHLMVStudioModelEntity.h"

#include "ui/assets/studiomodel/StudioModelContext.hpp"
#include "ui/assets/studiomodel/dockpanels/StudioModelModelInfoPanel.hpp"

namespace ui::assets::studiomodel
{
StudioModelModelInfoPanel::StudioModelModelInfoPanel(StudioModelContext* context, QWidget* parent)
	: QWidget(parent)
	, _context(context)
{
	_ui.setupUi(this);

	auto entity = _context->GetScene()->GetEntity();
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