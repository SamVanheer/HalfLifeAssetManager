#include <cassert>
#include <limits>

#include "ui/camera_operators/FreeLookCameraOperator.hpp"

#include "ui/camera_operators/dockpanels/FreeLookSettingsPanel.hpp"
#include "ui/camera_operators/dockpanels/CamerasPanel.hpp"

#include "utility/mathlib.hpp"

FreeLookSettingsPanel::FreeLookSettingsPanel(FreeLookCameraOperator* cameraOperator)
	: _cameraOperator(cameraOperator)
{
	assert(_cameraOperator);

	_ui.setupUi(this);

	QDoubleSpinBox* const spinBoxes[] =
	{
		_ui.OriginX,
		_ui.OriginY,
		_ui.OriginZ
	};

	for (auto spinBox : spinBoxes)
	{
		spinBox->setRange(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max());
	}

	setProperty(CameraOperatorPropertyKey.data(), QVariant::fromValue(static_cast<SceneCameraOperator*>(_cameraOperator)));

	UpdateCameraProperties();

	connect(_cameraOperator, &SceneCameraOperator::CameraPropertiesChanged, this, &FreeLookSettingsPanel::UpdateCameraProperties);

	connect(_ui.OriginX, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &FreeLookSettingsPanel::OnOriginChanged);
	connect(_ui.OriginY, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &FreeLookSettingsPanel::OnOriginChanged);
	connect(_ui.OriginZ, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &FreeLookSettingsPanel::OnOriginChanged);

	connect(_ui.Pitch, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &FreeLookSettingsPanel::OnAnglesChanged);
	connect(_ui.Yaw, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &FreeLookSettingsPanel::OnAnglesChanged);

	connect(_ui.FieldOfView, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &FreeLookSettingsPanel::OnFieldOfViewChanged);
	connect(_ui.DefaultFieldOfView, &QPushButton::clicked, this, &FreeLookSettingsPanel::OnResetFieldOfView);
}

FreeLookSettingsPanel::~FreeLookSettingsPanel() = default;

void FreeLookSettingsPanel::UpdateCameraProperties()
{
	const QSignalBlocker originX{_ui.OriginX};
	const QSignalBlocker originY{_ui.OriginY};
	const QSignalBlocker originZ{_ui.OriginZ};

	const QSignalBlocker pitch{_ui.Pitch};
	const QSignalBlocker yaw{_ui.Yaw};

	const QSignalBlocker fov{_ui.FieldOfView};

	auto camera = _cameraOperator->GetCamera();

	_ui.OriginX->setValue(camera->GetOrigin().x);
	_ui.OriginY->setValue(camera->GetOrigin().y);
	_ui.OriginZ->setValue(camera->GetOrigin().z);

	_ui.Pitch->setValue(FixAngle(camera->GetPitch()));
	_ui.Yaw->setValue(FixAngle(camera->GetYaw()));

	_ui.FieldOfView->setValue(camera->GetFieldOfView());
}

void FreeLookSettingsPanel::OnOriginChanged()
{
	_cameraOperator->SetOrigin({_ui.OriginX->value(), _ui.OriginY->value(), _ui.OriginZ->value()});
}

void FreeLookSettingsPanel::OnAnglesChanged()
{
	_cameraOperator->SetAngles(_ui.Pitch->value(), _ui.Yaw->value());
}

void FreeLookSettingsPanel::OnFieldOfViewChanged(double value)
{
	_cameraOperator->GetCamera()->SetFieldOfView(value);
}

void FreeLookSettingsPanel::OnResetFieldOfView()
{
	_ui.FieldOfView->setValue(FreeLookCameraOperator::DefaultFOV);
}
