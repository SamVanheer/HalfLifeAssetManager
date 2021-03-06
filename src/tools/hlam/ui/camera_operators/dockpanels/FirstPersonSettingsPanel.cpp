#include <cassert>

#include "ui/camera_operators/FirstPersonCameraOperator.hpp"

#include "ui/camera_operators/dockpanels/CamerasPanel.hpp"
#include "ui/camera_operators/dockpanels/FirstPersonSettingsPanel.hpp"

namespace ui::camera_operators
{
FirstPersonSettingsPanel::FirstPersonSettingsPanel(FirstPersonCameraOperator* cameraOperator, QWidget* parent)
	: QWidget(parent)
	, _cameraOperator(cameraOperator)
{
	assert(_cameraOperator);

	_ui.setupUi(this);

	setProperty(CameraOperatorPropertyKey.data(), QVariant::fromValue(static_cast<CameraOperator*>(_cameraOperator)));

	_ui.FieldOfView->setValue(_cameraOperator->GetCamera()->GetFieldOfView());

	connect(_ui.FieldOfView, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &FirstPersonSettingsPanel::OnFieldOfViewChanged);
	connect(_ui.DefaultFieldOfView, &QPushButton::clicked, this, &FirstPersonSettingsPanel::OnResetFieldOfView);
}

FirstPersonSettingsPanel::~FirstPersonSettingsPanel() = default;

void FirstPersonSettingsPanel::OnFieldOfViewChanged(double value)
{
	_cameraOperator->GetCamera()->SetFieldOfView(value);
}

void FirstPersonSettingsPanel::OnResetFieldOfView()
{
	_ui.FieldOfView->setValue(FirstPersonCameraOperator::DefaultFirstPersonFieldOfView);
}
}
