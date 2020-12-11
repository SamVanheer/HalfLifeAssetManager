#include "entity/CHLMVStudioModelEntity.h"

#include "ui/assets/studiomodel/StudioModelContext.hpp"
#include "ui/assets/studiomodel/dockpanels/StudioModelModelDisplayPanel.hpp"

namespace ui::assets::studiomodel
{
StudioModelModelDisplayPanel::StudioModelModelDisplayPanel(StudioModelContext* context, QWidget* parent)
	: QWidget(parent)
	, _context(context)
{
	_ui.setupUi(this);

	connect(_ui.RenderModeComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, &StudioModelModelDisplayPanel::OnRenderModeChanged);

	connect(_ui.OpacitySlider, &QSlider::valueChanged, this, &StudioModelModelDisplayPanel::OnOpacityChanged);

	connect(_ui.ShowHitboxes, &QCheckBox::stateChanged, this, &StudioModelModelDisplayPanel::OnShowHitboxesChanged);
	connect(_ui.ShowBones, &QCheckBox::stateChanged, this, &StudioModelModelDisplayPanel::OnShowBonesChanged);
	connect(_ui.ShowAttachments, &QCheckBox::stateChanged, this, &StudioModelModelDisplayPanel::OnShowAttachmentsChanged);
	connect(_ui.ShowEyePosition, &QCheckBox::stateChanged, this, &StudioModelModelDisplayPanel::OnShowEyePositionChanged);

	connect(_ui.BackfaceCulling, &QCheckBox::stateChanged, this, &StudioModelModelDisplayPanel::OnEnableBackfaceCullingChanged);
	connect(_ui.ShowGround, &QCheckBox::stateChanged, this, &StudioModelModelDisplayPanel::OnShowGroundChanged);
	connect(_ui.MirrorModelOnGround, &QCheckBox::stateChanged, this, &StudioModelModelDisplayPanel::OnMirrorOnGroundChanged);
	connect(_ui.ShowBackground, &QCheckBox::stateChanged, this, &StudioModelModelDisplayPanel::OnShowBackgroundChanged);

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

	connect(_ui.FOVSpinner, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &StudioModelModelDisplayPanel::OnFieldOfViewChanged);
	connect(_ui.ResetFOV, &QPushButton::clicked, this, &StudioModelModelDisplayPanel::OnResetFieldOfView);
	connect(_ui.FirstPersonFOVSpinner, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &StudioModelModelDisplayPanel::OnFirstPersonFieldOfViewChanged);
	connect(_ui.ResetFirstPersonFieldOfView, &QPushButton::clicked, this, &StudioModelModelDisplayPanel::OnResetFirstPersonFieldOfView);

	connect(_ui.CenterModelOnWorldOrigin, &QPushButton::clicked, this, &StudioModelModelDisplayPanel::OnCenterModelOnWorldOrigin);
	connect(_ui.AlignOnGround, &QPushButton::clicked, this, &StudioModelModelDisplayPanel::OnAlignOnGround);

	auto scene = _context->GetScene();

	_ui.FOVSpinner->setValue(scene->FieldOfView);
	_ui.FirstPersonFOVSpinner->setValue(scene->FirstPersonFieldOfView);
}

StudioModelModelDisplayPanel::~StudioModelModelDisplayPanel() = default;

void StudioModelModelDisplayPanel::OnRenderModeChanged(int index)
{
	_context->GetScene()->CurrentRenderMode = static_cast<RenderMode>(index);
}

void StudioModelModelDisplayPanel::OnOpacityChanged(int value)
{
	_context->GetScene()->GetEntity()->SetTransparency(value / 100.0f);

	_ui.OpacityValueLabel->setText(QString("%1%").arg(value));
}

void StudioModelModelDisplayPanel::OnShowHitboxesChanged()
{
	_context->GetScene()->ShowHitboxes = _ui.ShowHitboxes->isChecked();
}

void StudioModelModelDisplayPanel::OnShowBonesChanged()
{
	_context->GetScene()->ShowBones = _ui.ShowBones->isChecked();
}

void StudioModelModelDisplayPanel::OnShowAttachmentsChanged()
{
	_context->GetScene()->ShowAttachments = _ui.ShowAttachments->isChecked();
}

void StudioModelModelDisplayPanel::OnShowEyePositionChanged()
{
	_context->GetScene()->ShowEyePosition = _ui.ShowEyePosition->isChecked();
}

void StudioModelModelDisplayPanel::OnEnableBackfaceCullingChanged()
{
	_context->GetScene()->EnableBackfaceCulling = _ui.BackfaceCulling->isChecked();
}

void StudioModelModelDisplayPanel::OnShowGroundChanged()
{
	auto scene = _context->GetScene();

	scene->ShowGround = _ui.ShowGround->isChecked();

	if (!scene->ShowGround)
	{
		_ui.MirrorModelOnGround->setChecked(false);
	}
}

void StudioModelModelDisplayPanel::OnMirrorOnGroundChanged()
{
	auto scene = _context->GetScene();

	scene->MirrorOnGround = _ui.MirrorModelOnGround->isChecked();

	if (scene->MirrorOnGround)
	{
		_ui.ShowGround->setChecked(true);
	}
}

void StudioModelModelDisplayPanel::OnShowBackgroundChanged()
{
	_context->GetScene()->ShowBackground = _ui.ShowBackground->isChecked();
}

void StudioModelModelDisplayPanel::OnWireframeOverlayChanged()
{
	_context->GetScene()->ShowWireframeOverlay = _ui.WireframeOverlay->isChecked();
}

void StudioModelModelDisplayPanel::OnDrawShadowsChanged()
{
	_context->GetScene()->DrawShadows = _ui.DrawShadows->isChecked();
}

void StudioModelModelDisplayPanel::OnFixShadowZFightingChanged()
{
	_context->GetScene()->FixShadowZFighting = _ui.FixShadowZFighting->isChecked();
}

void StudioModelModelDisplayPanel::OnShowAxesChanged()
{
	_context->GetScene()->ShowAxes = _ui.ShowAxes->isChecked();
}

void StudioModelModelDisplayPanel::OnShowNormalsChanged()
{
	_context->GetScene()->ShowNormals = _ui.ShowNormals->isChecked();
}

void StudioModelModelDisplayPanel::OnShowCrosshairChanged()
{
	_context->GetScene()->ShowCrosshair = _ui.ShowCrosshair->isChecked();
}

void StudioModelModelDisplayPanel::OnShowGuidelinesChanged()
{
	_context->GetScene()->ShowGuidelines = _ui.ShowGuidelines->isChecked();
}

void StudioModelModelDisplayPanel::OnShowPlayerHitboxChanged()
{
	_context->GetScene()->ShowPlayerHitbox = _ui.ShowPlayerHitbox->isChecked();
}

void StudioModelModelDisplayPanel::OnMirrorXAxisChanged()
{
	_context->GetScene()->GetEntity()->GetScale().x = _ui.MirrorOnXAxis->isChecked() ? -1 : 1;
}

void StudioModelModelDisplayPanel::OnMirrorYAxisChanged()
{
	_context->GetScene()->GetEntity()->GetScale().y = _ui.MirrorOnYAxis->isChecked() ? -1 : 1;
}

void StudioModelModelDisplayPanel::OnMirrorZAxisChanged()
{
	_context->GetScene()->GetEntity()->GetScale().z = _ui.MirrorOnZAxis->isChecked() ? -1 : 1;
}

void StudioModelModelDisplayPanel::OnFieldOfViewChanged(double value)
{
	_context->GetScene()->FieldOfView = static_cast<float>(value);
}

void StudioModelModelDisplayPanel::OnResetFieldOfView()
{
	_ui.FOVSpinner->setValue(graphics::Scene::DefaultFOV);
}

void StudioModelModelDisplayPanel::OnFirstPersonFieldOfViewChanged(double value)
{
	_context->GetScene()->FirstPersonFieldOfView = static_cast<float>(value);
}

void StudioModelModelDisplayPanel::OnResetFirstPersonFieldOfView()
{
	_ui.FirstPersonFOVSpinner->setValue(graphics::Scene::DefaultFirstPersonFieldOfView);
}

void StudioModelModelDisplayPanel::OnCenterModelOnWorldOrigin()
{
	_context->GetScene()->GetEntity()->SetOrigin({0, 0, 0});
}

void StudioModelModelDisplayPanel::OnAlignOnGround()
{
	_context->GetScene()->AlignOnGround();
}
}
