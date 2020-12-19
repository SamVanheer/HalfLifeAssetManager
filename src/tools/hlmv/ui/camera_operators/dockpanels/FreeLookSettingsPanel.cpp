#include <cassert>

#include "ui/camera_operators/FreeLookCameraOperator.hpp"

#include "ui/camera_operators/dockpanels/FreeLookSettingsPanel.hpp"
#include "ui/camera_operators/dockpanels/CamerasPanel.hpp"

namespace ui::camera_operators
{
FreeLookSettingsPanel::FreeLookSettingsPanel(FreeLookCameraOperator* cameraOperator, QWidget* parent)
	: QWidget(parent)
	, _cameraOperator(cameraOperator)
{
	assert(_cameraOperator);

	_ui.setupUi(this);

	setProperty(CameraOperatorPropertyKey.data(), QVariant::fromValue(static_cast<CameraOperator*>(_cameraOperator)));

	_ui.FieldOfView->setValue(_cameraOperator->GetCamera()->GetFieldOfView());

	connect(_ui.FieldOfView, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &FreeLookSettingsPanel::OnFieldOfViewChanged);
	connect(_ui.DefaultFieldOfView, &QPushButton::clicked, this, &FreeLookSettingsPanel::OnResetFieldOfView);
	//TODO: hook up remaining properties once the coordinate system is fixed
}

FreeLookSettingsPanel::~FreeLookSettingsPanel() = default;

void FreeLookSettingsPanel::OnFieldOfViewChanged(double value)
{
	_cameraOperator->GetCamera()->SetFieldOfView(value);
}

void FreeLookSettingsPanel::OnResetFieldOfView()
{
	_ui.FieldOfView->setValue(FreeLookCameraOperator::DefaultFOV);
}
}
