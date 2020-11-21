#include "entity/CHLMVStudioModelEntity.h"

#include "ui/assets/studiomodel/StudioModelContext.hpp"
#include "ui/assets/studiomodel/dockpanels/StudioModelDisplayPanel.hpp"

namespace ui::assets::studiomodel
{
StudioModelDisplayPanel::StudioModelDisplayPanel(StudioModelContext* context, QWidget* parent)
	: QWidget(parent)
	, _context(context)
{
	_ui.setupUi(this);

	connect(_ui.RenderModeComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, &StudioModelDisplayPanel::OnRenderModeChanged);

	connect(_ui.OpacitySlider, &QSlider::valueChanged, this, &StudioModelDisplayPanel::OnOpacityChanged);

	connect(_ui.ShowHitboxes, &QCheckBox::stateChanged, this, &StudioModelDisplayPanel::OnShowHitboxesChanged);
	connect(_ui.ShowBones, &QCheckBox::stateChanged, this, &StudioModelDisplayPanel::OnShowBonesChanged);
	connect(_ui.ShowAttachments, &QCheckBox::stateChanged, this, &StudioModelDisplayPanel::OnShowAttachmentsChanged);
	connect(_ui.ShowEyePosition, &QCheckBox::stateChanged, this, &StudioModelDisplayPanel::OnShowEyePositionChanged);

	connect(_ui.BackfaceCulling, &QCheckBox::stateChanged, this, &StudioModelDisplayPanel::OnEnableBackfaceCullingChanged);
	connect(_ui.ShowGround, &QCheckBox::stateChanged, this, &StudioModelDisplayPanel::OnShowGroundChanged);
	connect(_ui.MirrorModelOnGround, &QCheckBox::stateChanged, this, &StudioModelDisplayPanel::OnMirrorOnGroundChanged);
	connect(_ui.ShowBackground, &QCheckBox::stateChanged, this, &StudioModelDisplayPanel::OnShowBackgroundChanged);

	connect(_ui.WireframeOverlay, &QCheckBox::stateChanged, this, &StudioModelDisplayPanel::OnWireframeOverlayChanged);
	connect(_ui.DrawShadows, &QCheckBox::stateChanged, this, &StudioModelDisplayPanel::OnDrawShadowsChanged);
	connect(_ui.FixShadowZFighting, &QCheckBox::stateChanged, this, &StudioModelDisplayPanel::OnFixShadowZFightingChanged);
	connect(_ui.ShowAxes, &QCheckBox::stateChanged, this, &StudioModelDisplayPanel::OnShowAxesChanged);

	connect(_ui.ShowNormals, &QCheckBox::stateChanged, this, &StudioModelDisplayPanel::OnShowNormalsChanged);
	connect(_ui.ShowCrosshair, &QCheckBox::stateChanged, this, &StudioModelDisplayPanel::OnShowCrosshairChanged);
	connect(_ui.ShowGuidelines, &QCheckBox::stateChanged, this, &StudioModelDisplayPanel::OnShowGuidelinesChanged);
	connect(_ui.ShowPlayerHitbox, &QCheckBox::stateChanged, this, &StudioModelDisplayPanel::OnShowPlayerHitboxChanged);

	connect(_ui.MirrorOnXAxis, &QCheckBox::stateChanged, this, &StudioModelDisplayPanel::OnMirrorXAxisChanged);
	connect(_ui.MirrorOnYAxis, &QCheckBox::stateChanged, this, &StudioModelDisplayPanel::OnMirrorYAxisChanged);
	connect(_ui.MirrorOnZAxis, &QCheckBox::stateChanged, this, &StudioModelDisplayPanel::OnMirrorZAxisChanged);

	connect(_ui.FOVSpinner, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &StudioModelDisplayPanel::OnFieldOfViewChanged);
	connect(_ui.ResetFOV, &QPushButton::clicked, this, &StudioModelDisplayPanel::OnResetFieldOfView);
	connect(_ui.FirstPersonFOVSpinner, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &StudioModelDisplayPanel::OnFirstPersonFieldOfViewChanged);
	connect(_ui.ResetFirstPersonFieldOfView, &QPushButton::clicked, this, &StudioModelDisplayPanel::OnResetFirstPersonFieldOfView);

	connect(_ui.CenterModelOnWorldOrigin, &QPushButton::clicked, this, &StudioModelDisplayPanel::OnCenterModelOnWorldOrigin);
	connect(_ui.AlignOnGround, &QPushButton::clicked, this, &StudioModelDisplayPanel::OnAlignOnGround);

	auto scene = _context->GetScene();

	_ui.FOVSpinner->setValue(scene->FieldOfView);
	_ui.FirstPersonFOVSpinner->setValue(scene->FirstPersonFieldOfView);
}

StudioModelDisplayPanel::~StudioModelDisplayPanel() = default;

void StudioModelDisplayPanel::OnRenderModeChanged(int index)
{
	_context->GetScene()->CurrentRenderMode = static_cast<RenderMode>(index);
}

void StudioModelDisplayPanel::OnOpacityChanged(int value)
{
	_context->GetScene()->GetEntity()->SetTransparency(value / 100.0f);

	_ui.OpacityValueLabel->setText(QString("%1%").arg(value));
}

void StudioModelDisplayPanel::OnShowHitboxesChanged()
{
	_context->GetScene()->ShowHitboxes = _ui.ShowHitboxes->isChecked();
}

void StudioModelDisplayPanel::OnShowBonesChanged()
{
	_context->GetScene()->ShowBones = _ui.ShowBones->isChecked();
}

void StudioModelDisplayPanel::OnShowAttachmentsChanged()
{
	_context->GetScene()->ShowAttachments = _ui.ShowAttachments->isChecked();
}

void StudioModelDisplayPanel::OnShowEyePositionChanged()
{
	_context->GetScene()->ShowEyePosition = _ui.ShowEyePosition->isChecked();
}

void StudioModelDisplayPanel::OnEnableBackfaceCullingChanged()
{
	_context->GetScene()->EnableBackfaceCulling = _ui.BackfaceCulling->isChecked();
}

void StudioModelDisplayPanel::OnShowGroundChanged()
{
	auto scene = _context->GetScene();

	scene->ShowGround = _ui.ShowGround->isChecked();

	if (!scene->ShowGround)
	{
		_ui.MirrorModelOnGround->setChecked(false);
	}
}

void StudioModelDisplayPanel::OnMirrorOnGroundChanged()
{
	auto scene = _context->GetScene();

	scene->MirrorOnGround = _ui.MirrorModelOnGround->isChecked();

	if (scene->MirrorOnGround)
	{
		_ui.ShowGround->setChecked(true);
	}
}

void StudioModelDisplayPanel::OnShowBackgroundChanged()
{
	_context->GetScene()->ShowBackground = _ui.ShowBackground->isChecked();
}

void StudioModelDisplayPanel::OnWireframeOverlayChanged()
{
	_context->GetScene()->ShowWireframeOverlay = _ui.WireframeOverlay->isChecked();
}

void StudioModelDisplayPanel::OnDrawShadowsChanged()
{
	_context->GetScene()->DrawShadows = _ui.DrawShadows->isChecked();
}

void StudioModelDisplayPanel::OnFixShadowZFightingChanged()
{
	_context->GetScene()->FixShadowZFighting = _ui.FixShadowZFighting->isChecked();
}

void StudioModelDisplayPanel::OnShowAxesChanged()
{
	_context->GetScene()->ShowAxes = _ui.ShowAxes->isChecked();
}

void StudioModelDisplayPanel::OnShowNormalsChanged()
{
	_context->GetScene()->ShowNormals = _ui.ShowNormals->isChecked();
}

void StudioModelDisplayPanel::OnShowCrosshairChanged()
{
	_context->GetScene()->ShowCrosshair = _ui.ShowCrosshair->isChecked();
}

void StudioModelDisplayPanel::OnShowGuidelinesChanged()
{
	_context->GetScene()->ShowGuidelines = _ui.ShowGuidelines->isChecked();
}

void StudioModelDisplayPanel::OnShowPlayerHitboxChanged()
{
	_context->GetScene()->ShowPlayerHitbox = _ui.ShowPlayerHitbox->isChecked();
}

void StudioModelDisplayPanel::OnMirrorXAxisChanged()
{
	_context->GetScene()->GetEntity()->GetScale().x = _ui.MirrorOnXAxis->isChecked() ? -1 : 1;
}

void StudioModelDisplayPanel::OnMirrorYAxisChanged()
{
	_context->GetScene()->GetEntity()->GetScale().y = _ui.MirrorOnYAxis->isChecked() ? -1 : 1;
}

void StudioModelDisplayPanel::OnMirrorZAxisChanged()
{
	_context->GetScene()->GetEntity()->GetScale().z = _ui.MirrorOnZAxis->isChecked() ? -1 : 1;
}

void StudioModelDisplayPanel::OnFieldOfViewChanged(double value)
{
	_context->GetScene()->FieldOfView = static_cast<float>(value);
}

void StudioModelDisplayPanel::OnResetFieldOfView()
{
	_ui.FOVSpinner->setValue(graphics::Scene::DefaultFOV);
}

void StudioModelDisplayPanel::OnFirstPersonFieldOfViewChanged(double value)
{
	_context->GetScene()->FirstPersonFieldOfView = static_cast<float>(value);
}

void StudioModelDisplayPanel::OnResetFirstPersonFieldOfView()
{
	_ui.FirstPersonFOVSpinner->setValue(graphics::Scene::DefaultFirstPersonFieldOfView);
}

void StudioModelDisplayPanel::OnCenterModelOnWorldOrigin()
{
	_context->GetScene()->GetEntity()->SetOrigin({0, 0, 0});
}

void StudioModelDisplayPanel::OnAlignOnGround()
{
	_context->GetScene()->AlignOnGround();
}
}
