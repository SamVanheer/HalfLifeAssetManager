#pragma once

#include <cassert>

#include <glm/trigonometric.hpp>
#include <glm/gtx/transform.hpp>

#include "ui/camera_operators/CameraOperator.hpp"
#include "ui/camera_operators/dockpanels/ArcBallSettingsPanel.hpp"
#include "ui/settings/GeneralSettings.hpp"

#include "utility/CoordinateSystem.hpp"

namespace ui::camera_operators
{
/**
*	@brief Maps user mouse input to a sphere that can be rotated
*/
class ArcBallCameraOperator : public CameraOperator
{
public:
	static constexpr float DefaultFOV = 65.f;

	ArcBallCameraOperator(settings::GeneralSettings* generalSettings)
		: _generalSettings(generalSettings)
	{
		assert(_generalSettings);

		_camera.SetFieldOfView(DefaultFOV);
	}

	~ArcBallCameraOperator() = default;

	QString GetName() const override
	{
		return "Arc Ball";
	}

	QWidget* CreateEditWidget() override
	{
		return new ArcBallSettingsPanel(this);
	}

	void MouseEvent(QMouseEvent& event) override
	{
		//TODO: needs all of the original functionality to be reimplemented
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

					_yaw -= horizontalAdjust;
					_pitch -= verticalAdjust;

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

					_distance += adjust;

					_oldCoordinates.x = event.x();
					_oldCoordinates.y = event.y();
				}

				UpdateArcBallCamera();
			}

			event.accept();
			break;
		}

		default: break;
		}
	}

	const glm::vec3& GetTargetPosition() const { return _targetPosition; }

	float GetPitch() const { return _pitch; }

	float GetYaw() const { return _yaw; }

	float GetDistance() const { return _distance; }

	void SetTargetPosition(const glm::vec3& targetPosition, float pitch, float yaw, float distance)
	{
		_targetPosition = targetPosition;
		_pitch = pitch;
		_yaw = yaw;
		_distance = distance;

		UpdateArcBallCamera();
	}

private:
	void UpdateArcBallCamera()
	{
		//First create a vector that represents a position at distance without any rotation applied
		auto cameraPosition = math::ForwardVector * -_distance;

		//Now apply rotation
		cameraPosition =
			glm::rotate(glm::radians(_yaw), math::UpVector) *
			glm::rotate(glm::radians(-_pitch), math::RightVector) *
			glm::vec4{cameraPosition, 1};

		//Make relative to target position
		cameraPosition += _targetPosition;

		//Set camera variables
		_camera.SetProperties(cameraPosition, _pitch, _yaw);

		emit CameraPropertiesChanged();
	}

private:
	const settings::GeneralSettings* const _generalSettings;

	glm::vec3 _targetPosition{0};

	float _pitch{0};
	float _yaw{0};
	float _distance{0};
};
}
