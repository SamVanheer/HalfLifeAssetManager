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
private:
	struct ArcBallParameters
	{
		glm::vec3 TargetPosition{0};

		float Pitch{0};
		float Yaw{0};
		float Distance{0};
	};

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

					_oldCoordinates.x = event.x();
					_oldCoordinates.y = event.y();

					if (event.modifiers() & Qt::KeyboardModifier::ShiftModifier)
					{
						//Apply input to YZ plane as target position movement
						_parameters.TargetPosition.y -= horizontalAdjust;
						_parameters.TargetPosition.z += verticalAdjust;
					}
					else
					{
						_parameters.Yaw -= horizontalAdjust;
						_parameters.Pitch -= verticalAdjust;
					}
				}
				else if (event.buttons() & Qt::MouseButton::RightButton)
				{
					auto adjust = static_cast<float>(event.y() - _oldCoordinates.y);

					if (_generalSettings->ShouldInvertMouseY())
					{
						adjust = -adjust;
					}

					_parameters.Distance += adjust;

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

	void CenterView(float height, float distance, float yaw) override
	{
		SetTargetPosition({0, 0, height}, 0, yaw, distance);
	}

	void SaveView() override
	{
		_savedParameters = _parameters;
	}

	void RestoreView() override
	{
		_parameters = _savedParameters;
		UpdateArcBallCamera();
	}

	const glm::vec3& GetTargetPosition() const { return _parameters.TargetPosition; }

	float GetPitch() const { return _parameters.Pitch; }

	float GetYaw() const { return _parameters.Yaw; }

	float GetDistance() const { return _parameters.Distance; }

	void SetTargetPosition(const glm::vec3& targetPosition, float pitch, float yaw, float distance)
	{
		_parameters.TargetPosition = targetPosition;
		_parameters.Pitch = pitch;
		_parameters.Yaw = yaw;
		_parameters.Distance = distance;

		UpdateArcBallCamera();
	}

private:
	void UpdateArcBallCamera()
	{
		//First create a vector that represents a position at distance without any rotation applied
		auto cameraPosition = math::ForwardVector * -_parameters.Distance;

		//Now apply rotation
		cameraPosition =
			glm::rotate(glm::radians(_parameters.Yaw), math::UpVector) *
			glm::rotate(glm::radians(-_parameters.Pitch), math::RightVector) *
			glm::vec4{cameraPosition, 1};

		//Make relative to target position
		cameraPosition += _parameters.TargetPosition;

		//Set camera variables
		_camera.SetProperties(cameraPosition, _parameters.Pitch, _parameters.Yaw);

		emit CameraPropertiesChanged();
	}

private:
	const settings::GeneralSettings* const _generalSettings;

	ArcBallParameters _parameters;
	ArcBallParameters _savedParameters;
};
}
