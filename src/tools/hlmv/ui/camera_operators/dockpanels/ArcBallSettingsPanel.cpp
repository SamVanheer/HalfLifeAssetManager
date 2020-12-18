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

	setProperty(CameraOperatorPropertyKey.data(), QVariant::fromValue(static_cast<CameraOperator*>(_cameraOperator)));

	_ui.FieldOfView->setValue(_cameraOperator->GetCamera()->GetFieldOfView());

	connect(_ui.FieldOfView, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &ArcBallSettingsPanel::OnFieldOfViewChanged);
	connect(_ui.DefaultFieldOfView, &QPushButton::clicked, this, &ArcBallSettingsPanel::OnResetFieldOfView);
	//TODO: hook up remaining properties once the coordinate system is fixed
}

ArcBallSettingsPanel::~ArcBallSettingsPanel() = default;

void ArcBallSettingsPanel::OnFieldOfViewChanged(double value)
{
	_cameraOperator->GetCamera()->SetFieldOfView(value);
}

void ArcBallSettingsPanel::OnResetFieldOfView()
{
	_ui.FieldOfView->setValue(ArcBallCameraOperator::DefaultFOV);
}
}
