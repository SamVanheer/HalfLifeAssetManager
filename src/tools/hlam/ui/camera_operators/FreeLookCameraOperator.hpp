#pragma once

#include "ui/camera_operators/CameraOperator.hpp"
#include "ui/camera_operators/dockpanels/FreeLookSettingsPanel.hpp"

#include "utility/CoordinateSystem.hpp"

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
		: CameraOperator(generalSettings)
	{
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
					const auto horizontalAdjust = GetMouseXValue(static_cast<float>(event.x() - _oldCoordinates.x));
					const auto verticalAdjust = GetMouseXValue(static_cast<float>(event.y() - _oldCoordinates.y));

					_oldCoordinates.x = event.x();
					_oldCoordinates.y = event.y();

					if (event.modifiers() & Qt::KeyboardModifier::ShiftModifier)
					{
						//Move the camera origin along the right and up vectors
						glm::vec3 origin = _camera.GetOrigin();

						origin += math::RightVector * horizontalAdjust;
						origin -= math::UpVector * verticalAdjust;

						SetOrigin(origin);
					}
					else
					{
						float pitch = _camera.GetPitch();
						float yaw = _camera.GetYaw();

						yaw -= horizontalAdjust;
						pitch -= verticalAdjust;

						SetAngles(pitch, yaw);
					}
				}
				else if (event.buttons() & Qt::MouseButton::RightButton)
				{
					const auto adjust = GetMouseYValue(static_cast<float>(event.y() - _oldCoordinates.y));

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

	void CenterView(float height, float distance, float yaw) override
	{
		_camera.SetOrigin({distance, 0, height});
		_camera.SetAngles(0, yaw);
		emit CameraPropertiesChanged();
	}

	void SaveView() override
	{
		_savedOrigin = _camera.GetOrigin();
		_savedPitch = _camera.GetPitch();
		_savedYaw = _camera.GetYaw();
	}

	void RestoreView() override
	{
		_camera.SetOrigin(_savedOrigin);
		_camera.SetAngles(_savedPitch, _savedYaw);
		emit CameraPropertiesChanged();
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
	glm::vec3 _savedOrigin{0};
	float _savedPitch{0};
	float _savedYaw{0};
};
}
