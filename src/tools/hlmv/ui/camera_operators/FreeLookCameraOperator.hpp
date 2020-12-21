#pragma once

#include <cassert>

#include "ui/camera_operators/CameraOperator.hpp"
#include "ui/camera_operators/dockpanels/FreeLookSettingsPanel.hpp"
#include "ui/settings/GeneralSettings.hpp"

namespace ui::camera_operators
{
/**
*	@brief Camera operator that allows the user to move in the 3D world freely using the mouse and keyboard
*/
class FreeLookCameraOperator final : public CameraOperator
{
public:
	static constexpr float DefaultFOV = 65.f;

	FreeLookCameraOperator(settings::GeneralSettings* generalSettings)
		: _generalSettings(generalSettings)
	{
		assert(_generalSettings);

		_camera.SetFieldOfView(DefaultFOV);
	}

	QString GetName() const override
	{
		return "Free Look";
	}

	QWidget* CreateEditWidget() override
	{
		return new FreeLookSettingsPanel(this);
	}

	void MouseEvent(QMouseEvent& event) override
	{
		//TODO: once the coordinate system is fixed this needs to be updated to work properly
		switch (event.type())
		{
		case QEvent::MouseButtonPress:
		{
			_oldCoordinates.x = event.x();
			_oldCoordinates.y = event.y();

			_trackedButtons |= event.button();

			event.accept();
			break;
		}

		case QEvent::MouseButtonRelease:
		{
			_trackedButtons &= ~event.button();
			event.accept();
			break;
		}

		case QEvent::MouseMove:
		{
			if (_trackedButtons & event.buttons())
			{
				if (event.buttons() & Qt::MouseButton::LeftButton)
				{
					float pitch = _camera.GetPitch();
					float yaw = _camera.GetYaw();

					auto horizontalAdjust = static_cast<float>(event.x() - _oldCoordinates.x);
					auto verticalAdjust = static_cast<float>(event.y() - _oldCoordinates.y);

					if (_generalSettings->ShouldInvertMouseX())
					{
						horizontalAdjust = -horizontalAdjust;
					}

					if (_generalSettings->ShouldInvertMouseY())
					{
						verticalAdjust = -verticalAdjust;
					}

					yaw -= horizontalAdjust;
					pitch -= verticalAdjust;

					SetAngles(pitch, yaw);

					_oldCoordinates.x = event.x();
					_oldCoordinates.y = event.y();
				}
				else if (event.buttons() & Qt::MouseButton::RightButton)
				{
					auto adjust = static_cast<float>(event.y() - _oldCoordinates.y);

					if (_generalSettings->ShouldInvertMouseY())
					{
						adjust = -adjust;
					}

					SetOrigin(_camera.GetOrigin() - (_camera.GetForwardVector() * adjust));

					_oldCoordinates.x = event.x();
					_oldCoordinates.y = event.y();
				}
			}

			event.accept();
			break;
		}

		default: break;
		}
	}

	void SetOrigin(const glm::vec3& origin)
	{
		_camera.SetOrigin(origin);
		emit CameraPropertiesChanged();
	}

	void SetAngles(float pitch, float yaw)
	{
		_camera.SetAngles(pitch, yaw);
		emit CameraPropertiesChanged();
	}

private:
	const settings::GeneralSettings* const _generalSettings;
};
}
