#include <QSignalBlocker>

#include "entity/AxesEntity.hpp"
#include "entity/BoundingBoxEntity.hpp"
#include "entity/ClippingBoxEntity.hpp"
#include "entity/CrosshairEntity.hpp"
#include "entity/GuidelinesEntity.hpp"
#include "entity/HLMVStudioModelEntity.hpp"
#include "entity/PlayerHitboxEntity.hpp"

#include "ui/assets/studiomodel/StudioModelAsset.hpp"
#include "ui/assets/studiomodel/dockpanels/ModelDisplayPanel.hpp"

namespace studiomodel
{
ModelDisplayPanel::ModelDisplayPanel(StudioModelAssetProvider* provider)
	: _provider(provider)
{
	_ui.setupUi(this);

	connect(_provider, &StudioModelAssetProvider::AssetChanged, this, &ModelDisplayPanel::OnAssetChanged);

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
}

ModelDisplayPanel::~ModelDisplayPanel() = default;

void ModelDisplayPanel::OnAssetChanged(StudioModelAsset* asset)
{
	if (_asset)
	{
		_asset->disconnect(this);
	}

	_asset = asset;

	const auto entity = _asset->GetEntity();

	const QSignalBlocker rendermodeBlocker{_ui.RenderModeComboBox};
	const QSignalBlocker hitboxes{_ui.ShowHitboxes};
	const QSignalBlocker bones{_ui.ShowBones};
	const QSignalBlocker attachments{_ui.ShowAttachments};
	const QSignalBlocker eyePosition{_ui.ShowEyePosition};
	const QSignalBlocker bbox{_ui.ShowBBox};
	const QSignalBlocker cbox{_ui.ShowCBox};
	const QSignalBlocker backfaceCulling{_ui.BackfaceCulling};
	const QSignalBlocker wireframe{_ui.WireframeOverlay};
	const QSignalBlocker shadows{_ui.DrawShadows};
	const QSignalBlocker shadowZFighting{_ui.FixShadowZFighting};
	const QSignalBlocker axes{_ui.ShowAxes};
	const QSignalBlocker normals{_ui.ShowNormals};
	const QSignalBlocker crosshair{_ui.ShowCrosshair};
	const QSignalBlocker guidelines{_ui.ShowGuidelines};
	const QSignalBlocker playerHitbox{_ui.ShowPlayerHitbox};
	const QSignalBlocker mirrorX{_ui.MirrorOnXAxis};
	const QSignalBlocker mirrorY{_ui.MirrorOnYAxis};
	const QSignalBlocker mirrorZ{_ui.MirrorOnZAxis};

	_ui.RenderModeComboBox->setCurrentIndex(static_cast<int>(_asset->CurrentRenderMode));
	_ui.OpacitySlider->setValue(static_cast<int>(entity->GetTransparency() * 100));
	_ui.ShowHitboxes->setChecked(_asset->ShowHitboxes);
	_ui.ShowBones->setChecked(_asset->ShowBones);
	_ui.ShowAttachments->setChecked(_asset->ShowAttachments);
	_ui.ShowEyePosition->setChecked(_asset->ShowEyePosition);
	_ui.ShowBBox->setChecked(_asset->GetBoundingBoxEntity()->ShowBBox);
	_ui.ShowCBox->setChecked(_asset->GetClippingBoxEntity()->ShowCBox);
	_ui.BackfaceCulling->setChecked(_asset->EnableBackfaceCulling);
	_ui.WireframeOverlay->setChecked(_asset->ShowWireframeOverlay);
	_ui.DrawShadows->setChecked(_asset->DrawShadows);
	_ui.FixShadowZFighting->setChecked(_asset->FixShadowZFighting);
	_ui.ShowAxes->setChecked(_asset->GetAxesEntity()->ShowAxes);
	_ui.ShowNormals->setChecked(_asset->ShowNormals);
	_ui.ShowCrosshair->setChecked(_asset->GetCrosshairEntity()->ShowCrosshair);
	_ui.ShowGuidelines->setChecked(_asset->GetGuidelinesEntity()->ShowGuidelines);
	_ui.ShowPlayerHitbox->setChecked(_asset->GetPlayerHitboxEntity()->ShowPlayerHitbox);

	_ui.MirrorOnXAxis->setChecked(entity->GetScale().x == -1);
	_ui.MirrorOnYAxis->setChecked(entity->GetScale().y == -1);
	_ui.MirrorOnZAxis->setChecked(entity->GetScale().z == -1);
}

void ModelDisplayPanel::OnRenderModeChanged(int index)
{
	_asset->CurrentRenderMode = static_cast<RenderMode>(index);
}

void ModelDisplayPanel::OnOpacityChanged(int value)
{
	_asset->GetEntity()->SetTransparency(value / 100.0f);

	_ui.OpacityValueLabel->setText(QString("%1%").arg(value));
}

void ModelDisplayPanel::OnShowHitboxesChanged()
{
	_asset->ShowHitboxes = _ui.ShowHitboxes->isChecked();
}

void ModelDisplayPanel::OnShowBonesChanged()
{
	_asset->ShowBones = _ui.ShowBones->isChecked();
}

void ModelDisplayPanel::OnShowAttachmentsChanged()
{
	_asset->ShowAttachments = _ui.ShowAttachments->isChecked();
}

void ModelDisplayPanel::OnShowEyePositionChanged()
{
	_asset->ShowEyePosition = _ui.ShowEyePosition->isChecked();
}

void ModelDisplayPanel::OnShowBBoxChanged()
{
	_asset->GetBoundingBoxEntity()->ShowBBox = _ui.ShowBBox->isChecked();
}

void ModelDisplayPanel::OnShowCBoxChanged()
{
	_asset->GetClippingBoxEntity()->ShowCBox = _ui.ShowCBox->isChecked();
}

void ModelDisplayPanel::OnEnableBackfaceCullingChanged()
{
	_asset->EnableBackfaceCulling = _ui.BackfaceCulling->isChecked();
}

void ModelDisplayPanel::OnWireframeOverlayChanged()
{
	_asset->ShowWireframeOverlay = _ui.WireframeOverlay->isChecked();
}

void ModelDisplayPanel::OnDrawShadowsChanged()
{
	_asset->DrawShadows = _ui.DrawShadows->isChecked();
}

void ModelDisplayPanel::OnFixShadowZFightingChanged()
{
	_asset->FixShadowZFighting = _ui.FixShadowZFighting->isChecked();
}

void ModelDisplayPanel::OnShowAxesChanged()
{
	_asset->GetAxesEntity()->ShowAxes = _ui.ShowAxes->isChecked();
}

void ModelDisplayPanel::OnShowNormalsChanged()
{
	_asset->ShowNormals = _ui.ShowNormals->isChecked();
}

void ModelDisplayPanel::OnShowCrosshairChanged()
{
	_asset->GetCrosshairEntity()->ShowCrosshair = _ui.ShowCrosshair->isChecked();
}

void ModelDisplayPanel::OnShowGuidelinesChanged()
{
	_asset->GetGuidelinesEntity()->ShowGuidelines = _ui.ShowGuidelines->isChecked();
}

void ModelDisplayPanel::OnShowPlayerHitboxChanged()
{
	_asset->GetPlayerHitboxEntity()->ShowPlayerHitbox = _ui.ShowPlayerHitbox->isChecked();
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
