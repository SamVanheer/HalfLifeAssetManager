#pragma once

#include "ui/camera_operators/CameraOperator.hpp"
#include "ui/camera_operators/dockpanels/FirstPersonSettingsPanel.hpp"

namespace ui::camera_operators
{
/**
*	@brief Camera operator that allows the user to view an object in first person view
*/
class FirstPersonCameraOperator final : public CameraOperator
{
public:
	static constexpr float DefaultFirstPersonFieldOfView = 74.f;

	FirstPersonCameraOperator(settings::GeneralSettings* generalSettings)
		: CameraOperator(generalSettings)
	{
		_camera.SetOrigin(glm::vec3{0});
		_camera.SetFieldOfView(DefaultFirstPersonFieldOfView);
	}

	QString GetName() const override
	{
		return "First Person";
	}

	QWidget* CreateEditWidget() override
	{
		return new FirstPersonSettingsPanel(this);
	}

	void MouseEvent(QMouseEvent& event) override
	{
		//No movement of any kind
		event.accept();
	}

	void CenterView(float, float, float) override {}

	void SaveView() override {}

	void RestoreView() override {}
};
}
