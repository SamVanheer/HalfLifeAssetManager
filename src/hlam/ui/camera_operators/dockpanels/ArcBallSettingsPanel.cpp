#include <cassert>
#include <limits>

#include "ui/camera_operators/ArcBallCameraOperator.hpp"

#include "ui/camera_operators/dockpanels/ArcBallSettingsPanel.hpp"
#include "ui/camera_operators/dockpanels/CamerasPanel.hpp"

ArcBallSettingsPanel::ArcBallSettingsPanel(ArcBallCameraOperator* cameraOperator)
	: _cameraOperator(cameraOperator)
{
	assert(_cameraOperator);

	_ui.setupUi(this);

	_ui.Distance->setRange(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max());

	setProperty(CameraOperatorPropertyKey.data(), QVariant::fromValue(static_cast<SceneCameraOperator*>(_cameraOperator)));

	UpdateCameraProperties();

	connect(_cameraOperator, &SceneCameraOperator::CameraPropertiesChanged, this, &ArcBallSettingsPanel::UpdateCameraProperties);

	connect(_ui.FieldOfView, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &ArcBallSettingsPanel::OnFieldOfViewChanged);
	connect(_ui.DefaultFieldOfView, &QPushButton::clicked, this, &ArcBallSettingsPanel::OnResetFieldOfView);

	connect(_ui.Pitch, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &ArcBallSettingsPanel::OnPropertyChanged);
	connect(_ui.Yaw, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &ArcBallSettingsPanel::OnPropertyChanged);
	connect(_ui.Center, &qt::widgets::SimpleVector3Edit::ValueChanged, this, &ArcBallSettingsPanel::OnPropertyChanged);
	connect(_ui.Distance, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &ArcBallSettingsPanel::OnPropertyChanged);

	connect(_ui.ProjectionMode, qOverload<int>(&QComboBox::currentIndexChanged),
		this, &ArcBallSettingsPanel::OnProjectionModeChanged);
}

ArcBallSettingsPanel::~ArcBallSettingsPanel() = default;

void ArcBallSettingsPanel::UpdateCameraProperties()
{
	const QSignalBlocker fov{_ui.FieldOfView};
	const QSignalBlocker center{_ui.Center};
	const QSignalBlocker pitch{_ui.Pitch};
	const QSignalBlocker yaw{_ui.Yaw};
	const QSignalBlocker distance{_ui.Distance};
	const QSignalBlocker projectionmode{_ui.ProjectionMode};

	auto camera = _cameraOperator->GetCamera();

	_ui.FieldOfView->setValue(camera->GetFieldOfView());
	_ui.Pitch->setValue(FixAngle(_cameraOperator->GetPitch()));
	_ui.Yaw->setValue(FixAngle(_cameraOperator->GetYaw()));
	_ui.Center->SetValue(_cameraOperator->GetTargetPosition());
	_ui.Distance->setValue(_cameraOperator->GetDistance());
	_ui.ProjectionMode->setCurrentIndex(static_cast<int>(camera->GetProjectionMode()));
}

void ArcBallSettingsPanel::OnFieldOfViewChanged(double value)
{
	_cameraOperator->GetCamera()->SetFieldOfView(value);
}

void ArcBallSettingsPanel::OnResetFieldOfView()
{
	_ui.FieldOfView->setValue(ArcBallCameraOperator::DefaultFOV);
}

void ArcBallSettingsPanel::OnPropertyChanged()
{
	_cameraOperator->SetTargetPosition(
		_ui.Center->GetValue(), _ui.Pitch->value(), _ui.Yaw->value(), _ui.Distance->value());
}

void ArcBallSettingsPanel::OnProjectionModeChanged(int index)
{
	_cameraOperator->GetCamera()->SetProjectionMode(static_cast<graphics::ProjectionMode>(index));
}
