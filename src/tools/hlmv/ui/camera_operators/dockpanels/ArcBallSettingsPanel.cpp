#include <cassert>

#include "ui/camera_operators/ArcBallCameraOperator.hpp"

#include "ui/camera_operators/dockpanels/ArcBallSettingsPanel.hpp"
#include "ui/camera_operators/dockpanels/CamerasPanel.hpp"

namespace ui::camera_operators
{
ArcBallSettingsPanel::ArcBallSettingsPanel(ArcBallCameraOperator* cameraOperator, QWidget* parent)
	: QWidget(parent)
	, _cameraOperator(cameraOperator)
{
	assert(_cameraOperator);

	_ui.setupUi(this);

	QDoubleSpinBox* const spinBoxes[] =
	{
		_ui.CenterX,
		_ui.CenterY,
		_ui.CenterZ
	};

	for (auto spinBox : spinBoxes)
	{
		spinBox->setRange(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max());
	}

	_ui.Distance->setRange(0, std::numeric_limits<double>::max());

	setProperty(CameraOperatorPropertyKey.data(), QVariant::fromValue(static_cast<CameraOperator*>(_cameraOperator)));

	UpdateCameraProperties();

	connect(_cameraOperator, &CameraOperator::CameraPropertiesChanged, this, &ArcBallSettingsPanel::UpdateCameraProperties);

	connect(_ui.CenterX, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &ArcBallSettingsPanel::OnPropertyChanged);
	connect(_ui.CenterY, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &ArcBallSettingsPanel::OnPropertyChanged);
	connect(_ui.CenterZ, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &ArcBallSettingsPanel::OnPropertyChanged);

	connect(_ui.Pitch, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &ArcBallSettingsPanel::OnPropertyChanged);
	connect(_ui.Yaw, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &ArcBallSettingsPanel::OnPropertyChanged);

	connect(_ui.Distance, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &ArcBallSettingsPanel::OnPropertyChanged);

	connect(_ui.FieldOfView, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &ArcBallSettingsPanel::OnFieldOfViewChanged);
	connect(_ui.DefaultFieldOfView, &QPushButton::clicked, this, &ArcBallSettingsPanel::OnResetFieldOfView);
}

ArcBallSettingsPanel::~ArcBallSettingsPanel() = default;

void ArcBallSettingsPanel::UpdateCameraProperties()
{
	const QSignalBlocker centerX{_ui.CenterX};
	const QSignalBlocker centerY{_ui.CenterY};
	const QSignalBlocker centerZ{_ui.CenterZ};

	const QSignalBlocker pitch{_ui.Pitch};
	const QSignalBlocker yaw{_ui.Yaw};

	const QSignalBlocker fov{_ui.FieldOfView};

	auto camera = _cameraOperator->GetCamera();

	_ui.CenterX->setValue(_cameraOperator->GetTargetPosition().x);
	_ui.CenterY->setValue(_cameraOperator->GetTargetPosition().y);
	_ui.CenterZ->setValue(_cameraOperator->GetTargetPosition().z);

	_ui.Pitch->setValue(FixAngle(_cameraOperator->GetPitch()));
	_ui.Yaw->setValue(FixAngle(_cameraOperator->GetYaw()));

	_ui.Distance->setValue(_cameraOperator->GetDistance());

	_ui.FieldOfView->setValue(camera->GetFieldOfView());
}

void ArcBallSettingsPanel::OnPropertyChanged()
{
	_cameraOperator->SetTargetPosition(
		{_ui.CenterX->value(), _ui.CenterY->value(), _ui.CenterZ->value()}, _ui.Pitch->value(), _ui.Yaw->value(), _ui.Distance->value());
}

void ArcBallSettingsPanel::OnFieldOfViewChanged(double value)
{
	_cameraOperator->GetCamera()->SetFieldOfView(value);
}

void ArcBallSettingsPanel::OnResetFieldOfView()
{
	_ui.FieldOfView->setValue(ArcBallCameraOperator::DefaultFOV);
}
}
