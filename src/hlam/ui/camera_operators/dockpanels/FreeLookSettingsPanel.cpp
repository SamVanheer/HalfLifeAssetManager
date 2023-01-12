#include <cassert>

#include "ui/camera_operators/FreeLookCameraOperator.hpp"

#include "ui/camera_operators/dockpanels/FreeLookSettingsPanel.hpp"
#include "ui/camera_operators/dockpanels/CamerasPanel.hpp"

#include "utility/mathlib.hpp"

FreeLookSettingsPanel::FreeLookSettingsPanel(FreeLookCameraOperator* cameraOperator)
	: _cameraOperator(cameraOperator)
{
	assert(_cameraOperator);

	_ui.setupUi(this);

	setProperty(CameraOperatorPropertyKey.data(), QVariant::fromValue(static_cast<SceneCameraOperator*>(_cameraOperator)));

	UpdateCameraProperties();

	connect(_cameraOperator, &SceneCameraOperator::CameraPropertiesChanged, this, &FreeLookSettingsPanel::UpdateCameraProperties);

	connect(_ui.FieldOfView, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &FreeLookSettingsPanel::OnFieldOfViewChanged);
	connect(_ui.DefaultFieldOfView, &QPushButton::clicked, this, &FreeLookSettingsPanel::OnResetFieldOfView);

	connect(_ui.Pitch, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &FreeLookSettingsPanel::OnAnglesChanged);
	connect(_ui.Yaw, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &FreeLookSettingsPanel::OnAnglesChanged);

	connect(_ui.Origin, &qt::widgets::SimpleVector3Edit::ValueChanged, this, &FreeLookSettingsPanel::OnOriginChanged);
}

FreeLookSettingsPanel::~FreeLookSettingsPanel() = default;

void FreeLookSettingsPanel::UpdateCameraProperties()
{
	const QSignalBlocker origin{_ui.Origin};

	const QSignalBlocker pitch{_ui.Pitch};
	const QSignalBlocker yaw{_ui.Yaw};

	const QSignalBlocker fov{_ui.FieldOfView};

	auto camera = _cameraOperator->GetCamera();

	_ui.Origin->SetValue(camera->GetOrigin());

	_ui.Pitch->setValue(FixAngle(camera->GetPitch()));
	_ui.Yaw->setValue(FixAngle(camera->GetYaw()));

	_ui.FieldOfView->setValue(camera->GetFieldOfView());
}

void FreeLookSettingsPanel::OnFieldOfViewChanged(double value)
{
	_cameraOperator->GetCamera()->SetFieldOfView(value);
}

void FreeLookSettingsPanel::OnResetFieldOfView()
{
	_ui.FieldOfView->setValue(FreeLookCameraOperator::DefaultFOV);
}

void FreeLookSettingsPanel::OnAnglesChanged()
{
	_cameraOperator->SetAngles(_ui.Pitch->value(), _ui.Yaw->value());
}


void FreeLookSettingsPanel::OnOriginChanged()
{
	_cameraOperator->SetOrigin(_ui.Origin->GetValue());
}
