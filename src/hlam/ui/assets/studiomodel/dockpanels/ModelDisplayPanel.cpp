#include "entity/HLMVStudioModelEntity.hpp"

#include "ui/assets/studiomodel/StudioModelAsset.hpp"
#include "ui/assets/studiomodel/dockpanels/ModelDisplayPanel.hpp"

#include "ui/settings/StudioModelSettings.hpp"

namespace ui::assets::studiomodel
{
ModelDisplayPanel::ModelDisplayPanel(StudioModelAsset* asset)
	: _asset(asset)
{
	_ui.setupUi(this);

	connect(_ui.RenderModeComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, &ModelDisplayPanel::OnRenderModeChanged);

	connect(_ui.OpacitySlider, &QSlider::valueChanged, this, &ModelDisplayPanel::OnOpacityChanged);

	connect(_ui.ShowHitboxes, &QCheckBox::stateChanged, this, &ModelDisplayPanel::OnShowHitboxesChanged);
	connect(_ui.ShowBones, &QCheckBox::stateChanged, this, &ModelDisplayPanel::OnShowBonesChanged);
	connect(_ui.ShowAttachments, &QCheckBox::stateChanged, this, &ModelDisplayPanel::OnShowAttachmentsChanged);
	connect(_ui.ShowEyePosition, &QCheckBox::stateChanged, this, &ModelDisplayPanel::OnShowEyePositionChanged);

	connect(_ui.ShowBBox, &QCheckBox::stateChanged, this, &ModelDisplayPanel::OnShowBBoxChanged);
	connect(_ui.ShowCBox, &QCheckBox::stateChanged, this, &ModelDisplayPanel::OnShowCBoxChanged);

	connect(_ui.BackfaceCulling, &QCheckBox::stateChanged, this, &ModelDisplayPanel::OnEnableBackfaceCullingChanged);

	connect(_ui.WireframeOverlay, &QCheckBox::stateChanged, this, &ModelDisplayPanel::OnWireframeOverlayChanged);
	connect(_ui.DrawShadows, &QCheckBox::stateChanged, this, &ModelDisplayPanel::OnDrawShadowsChanged);
	connect(_ui.FixShadowZFighting, &QCheckBox::stateChanged, this, &ModelDisplayPanel::OnFixShadowZFightingChanged);
	connect(_ui.ShowAxes, &QCheckBox::stateChanged, this, &ModelDisplayPanel::OnShowAxesChanged);

	connect(_ui.ShowNormals, &QCheckBox::stateChanged, this, &ModelDisplayPanel::OnShowNormalsChanged);
	connect(_ui.ShowCrosshair, &QCheckBox::stateChanged, this, &ModelDisplayPanel::OnShowCrosshairChanged);
	connect(_ui.ShowGuidelines, &QCheckBox::stateChanged, this, &ModelDisplayPanel::OnShowGuidelinesChanged);
	connect(_ui.ShowPlayerHitbox, &QCheckBox::stateChanged, this, &ModelDisplayPanel::OnShowPlayerHitboxChanged);

	connect(_ui.MirrorOnXAxis, &QCheckBox::stateChanged, this, &ModelDisplayPanel::OnMirrorXAxisChanged);
	connect(_ui.MirrorOnYAxis, &QCheckBox::stateChanged, this, &ModelDisplayPanel::OnMirrorYAxisChanged);
	connect(_ui.MirrorOnZAxis, &QCheckBox::stateChanged, this, &ModelDisplayPanel::OnMirrorZAxisChanged);

	_ui.RenderModeComboBox->setCurrentIndex(static_cast<int>(_asset->GetProvider()->GetStudioModelSettings()->CurrentRenderMode));
}

ModelDisplayPanel::~ModelDisplayPanel() = default;

void ModelDisplayPanel::OnRenderModeChanged(int index)
{
	_asset->GetProvider()->GetStudioModelSettings()->CurrentRenderMode = static_cast<RenderMode>(index);
}

void ModelDisplayPanel::OnOpacityChanged(int value)
{
	_asset->GetEntity()->SetTransparency(value / 100.0f);

	_ui.OpacityValueLabel->setText(QString("%1%").arg(value));
}

void ModelDisplayPanel::OnShowHitboxesChanged()
{
	_asset->GetProvider()->GetStudioModelSettings()->ShowHitboxes = _ui.ShowHitboxes->isChecked();
}

void ModelDisplayPanel::OnShowBonesChanged()
{
	_asset->GetProvider()->GetStudioModelSettings()->ShowBones = _ui.ShowBones->isChecked();
}

void ModelDisplayPanel::OnShowAttachmentsChanged()
{
	_asset->GetProvider()->GetStudioModelSettings()->ShowAttachments = _ui.ShowAttachments->isChecked();
}

void ModelDisplayPanel::OnShowEyePositionChanged()
{
	_asset->GetProvider()->GetStudioModelSettings()->ShowEyePosition = _ui.ShowEyePosition->isChecked();
}

void ModelDisplayPanel::OnShowBBoxChanged()
{
	_asset->GetProvider()->GetStudioModelSettings()->ShowBBox = _ui.ShowBBox->isChecked();
}

void ModelDisplayPanel::OnShowCBoxChanged()
{
	_asset->GetProvider()->GetStudioModelSettings()->ShowCBox = _ui.ShowCBox->isChecked();
}

void ModelDisplayPanel::OnEnableBackfaceCullingChanged()
{
	_asset->GetProvider()->GetStudioModelSettings()->EnableBackfaceCulling = _ui.BackfaceCulling->isChecked();
}

void ModelDisplayPanel::OnWireframeOverlayChanged()
{
	_asset->GetProvider()->GetStudioModelSettings()->ShowWireframeOverlay = _ui.WireframeOverlay->isChecked();
}

void ModelDisplayPanel::OnDrawShadowsChanged()
{
	_asset->GetProvider()->GetStudioModelSettings()->DrawShadows = _ui.DrawShadows->isChecked();
}

void ModelDisplayPanel::OnFixShadowZFightingChanged()
{
	_asset->GetProvider()->GetStudioModelSettings()->FixShadowZFighting = _ui.FixShadowZFighting->isChecked();
}

void ModelDisplayPanel::OnShowAxesChanged()
{
	_asset->GetProvider()->GetStudioModelSettings()->ShowAxes = _ui.ShowAxes->isChecked();
}

void ModelDisplayPanel::OnShowNormalsChanged()
{
	_asset->GetProvider()->GetStudioModelSettings()->ShowNormals = _ui.ShowNormals->isChecked();
}

void ModelDisplayPanel::OnShowCrosshairChanged()
{
	_asset->GetProvider()->GetStudioModelSettings()->ShowCrosshair = _ui.ShowCrosshair->isChecked();
}

void ModelDisplayPanel::OnShowGuidelinesChanged()
{
	_asset->GetProvider()->GetStudioModelSettings()->ShowGuidelines = _ui.ShowGuidelines->isChecked();
}

void ModelDisplayPanel::OnShowPlayerHitboxChanged()
{
	_asset->GetProvider()->GetStudioModelSettings()->ShowPlayerHitbox = _ui.ShowPlayerHitbox->isChecked();
}

void ModelDisplayPanel::OnMirrorXAxisChanged()
{
	_asset->GetEntity()->GetScale().x = _ui.MirrorOnXAxis->isChecked() ? -1 : 1;
}

void ModelDisplayPanel::OnMirrorYAxisChanged()
{
	_asset->GetEntity()->GetScale().y = _ui.MirrorOnYAxis->isChecked() ? -1 : 1;
}

void ModelDisplayPanel::OnMirrorZAxisChanged()
{
	_asset->GetEntity()->GetScale().z = _ui.MirrorOnZAxis->isChecked() ? -1 : 1;
}
}
