#include "entity/HLMVStudioModelEntity.hpp"

#include "ui/assets/studiomodel/StudioModelAsset.hpp"
#include "ui/assets/studiomodel/dockpanels/StudioModelModelDisplayPanel.hpp"

#include "ui/settings/StudioModelSettings.hpp"

namespace ui::assets::studiomodel
{
StudioModelModelDisplayPanel::StudioModelModelDisplayPanel(StudioModelAsset* asset)
	: _asset(asset)
{
	_ui.setupUi(this);

	connect(_ui.RenderModeComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, &StudioModelModelDisplayPanel::OnRenderModeChanged);

	connect(_ui.OpacitySlider, &QSlider::valueChanged, this, &StudioModelModelDisplayPanel::OnOpacityChanged);

	connect(_ui.ShowHitboxes, &QCheckBox::stateChanged, this, &StudioModelModelDisplayPanel::OnShowHitboxesChanged);
	connect(_ui.ShowBones, &QCheckBox::stateChanged, this, &StudioModelModelDisplayPanel::OnShowBonesChanged);
	connect(_ui.ShowAttachments, &QCheckBox::stateChanged, this, &StudioModelModelDisplayPanel::OnShowAttachmentsChanged);
	connect(_ui.ShowEyePosition, &QCheckBox::stateChanged, this, &StudioModelModelDisplayPanel::OnShowEyePositionChanged);

	connect(_ui.ShowBBox, &QCheckBox::stateChanged, this, &StudioModelModelDisplayPanel::OnShowBBoxChanged);
	connect(_ui.ShowCBox, &QCheckBox::stateChanged, this, &StudioModelModelDisplayPanel::OnShowCBoxChanged);

	connect(_ui.BackfaceCulling, &QCheckBox::stateChanged, this, &StudioModelModelDisplayPanel::OnEnableBackfaceCullingChanged);

	connect(_ui.WireframeOverlay, &QCheckBox::stateChanged, this, &StudioModelModelDisplayPanel::OnWireframeOverlayChanged);
	connect(_ui.DrawShadows, &QCheckBox::stateChanged, this, &StudioModelModelDisplayPanel::OnDrawShadowsChanged);
	connect(_ui.FixShadowZFighting, &QCheckBox::stateChanged, this, &StudioModelModelDisplayPanel::OnFixShadowZFightingChanged);
	connect(_ui.ShowAxes, &QCheckBox::stateChanged, this, &StudioModelModelDisplayPanel::OnShowAxesChanged);

	connect(_ui.ShowNormals, &QCheckBox::stateChanged, this, &StudioModelModelDisplayPanel::OnShowNormalsChanged);
	connect(_ui.ShowCrosshair, &QCheckBox::stateChanged, this, &StudioModelModelDisplayPanel::OnShowCrosshairChanged);
	connect(_ui.ShowGuidelines, &QCheckBox::stateChanged, this, &StudioModelModelDisplayPanel::OnShowGuidelinesChanged);
	connect(_ui.ShowPlayerHitbox, &QCheckBox::stateChanged, this, &StudioModelModelDisplayPanel::OnShowPlayerHitboxChanged);

	connect(_ui.MirrorOnXAxis, &QCheckBox::stateChanged, this, &StudioModelModelDisplayPanel::OnMirrorXAxisChanged);
	connect(_ui.MirrorOnYAxis, &QCheckBox::stateChanged, this, &StudioModelModelDisplayPanel::OnMirrorYAxisChanged);
	connect(_ui.MirrorOnZAxis, &QCheckBox::stateChanged, this, &StudioModelModelDisplayPanel::OnMirrorZAxisChanged);

	_ui.RenderModeComboBox->setCurrentIndex(static_cast<int>(_asset->GetProvider()->GetStudioModelSettings()->CurrentRenderMode));
}

StudioModelModelDisplayPanel::~StudioModelModelDisplayPanel() = default;

void StudioModelModelDisplayPanel::OnRenderModeChanged(int index)
{
	_asset->GetProvider()->GetStudioModelSettings()->CurrentRenderMode = static_cast<RenderMode>(index);
}

void StudioModelModelDisplayPanel::OnOpacityChanged(int value)
{
	_asset->GetEntity()->SetTransparency(value / 100.0f);

	_ui.OpacityValueLabel->setText(QString("%1%").arg(value));
}

void StudioModelModelDisplayPanel::OnShowHitboxesChanged()
{
	_asset->GetProvider()->GetStudioModelSettings()->ShowHitboxes = _ui.ShowHitboxes->isChecked();
}

void StudioModelModelDisplayPanel::OnShowBonesChanged()
{
	_asset->GetProvider()->GetStudioModelSettings()->ShowBones = _ui.ShowBones->isChecked();
}

void StudioModelModelDisplayPanel::OnShowAttachmentsChanged()
{
	_asset->GetProvider()->GetStudioModelSettings()->ShowAttachments = _ui.ShowAttachments->isChecked();
}

void StudioModelModelDisplayPanel::OnShowEyePositionChanged()
{
	_asset->GetProvider()->GetStudioModelSettings()->ShowEyePosition = _ui.ShowEyePosition->isChecked();
}

void StudioModelModelDisplayPanel::OnShowBBoxChanged()
{
	_asset->GetProvider()->GetStudioModelSettings()->ShowBBox = _ui.ShowBBox->isChecked();
}

void StudioModelModelDisplayPanel::OnShowCBoxChanged()
{
	_asset->GetProvider()->GetStudioModelSettings()->ShowCBox = _ui.ShowCBox->isChecked();
}

void StudioModelModelDisplayPanel::OnEnableBackfaceCullingChanged()
{
	_asset->GetProvider()->GetStudioModelSettings()->EnableBackfaceCulling = _ui.BackfaceCulling->isChecked();
}

void StudioModelModelDisplayPanel::OnWireframeOverlayChanged()
{
	_asset->GetProvider()->GetStudioModelSettings()->ShowWireframeOverlay = _ui.WireframeOverlay->isChecked();
}

void StudioModelModelDisplayPanel::OnDrawShadowsChanged()
{
	_asset->GetProvider()->GetStudioModelSettings()->DrawShadows = _ui.DrawShadows->isChecked();
}

void StudioModelModelDisplayPanel::OnFixShadowZFightingChanged()
{
	_asset->GetProvider()->GetStudioModelSettings()->FixShadowZFighting = _ui.FixShadowZFighting->isChecked();
}

void StudioModelModelDisplayPanel::OnShowAxesChanged()
{
	_asset->GetProvider()->GetStudioModelSettings()->ShowAxes = _ui.ShowAxes->isChecked();
}

void StudioModelModelDisplayPanel::OnShowNormalsChanged()
{
	_asset->GetProvider()->GetStudioModelSettings()->ShowNormals = _ui.ShowNormals->isChecked();
}

void StudioModelModelDisplayPanel::OnShowCrosshairChanged()
{
	_asset->GetProvider()->GetStudioModelSettings()->ShowCrosshair = _ui.ShowCrosshair->isChecked();
}

void StudioModelModelDisplayPanel::OnShowGuidelinesChanged()
{
	_asset->GetProvider()->GetStudioModelSettings()->ShowGuidelines = _ui.ShowGuidelines->isChecked();
}

void StudioModelModelDisplayPanel::OnShowPlayerHitboxChanged()
{
	_asset->GetProvider()->GetStudioModelSettings()->ShowPlayerHitbox = _ui.ShowPlayerHitbox->isChecked();
}

void StudioModelModelDisplayPanel::OnMirrorXAxisChanged()
{
	_asset->GetEntity()->GetScale().x = _ui.MirrorOnXAxis->isChecked() ? -1 : 1;
}

void StudioModelModelDisplayPanel::OnMirrorYAxisChanged()
{
	_asset->GetEntity()->GetScale().y = _ui.MirrorOnYAxis->isChecked() ? -1 : 1;
}

void StudioModelModelDisplayPanel::OnMirrorZAxisChanged()
{
	_asset->GetEntity()->GetScale().z = _ui.MirrorOnZAxis->isChecked() ? -1 : 1;
}
}
