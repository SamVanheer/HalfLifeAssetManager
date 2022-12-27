#pragma once

#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>
#include <glm/gtx/transform.hpp>

#include "ui/camera_operators/CameraOperator.hpp"
#include "ui/camera_operators/dockpanels/ArcBallSettingsPanel.hpp"

#include "utility/CoordinateSystem.hpp"

/**
*	@brief Maps user mouse input to a sphere that can be rotated
*/
class ArcBallCameraOperator : public SceneCameraOperator
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

	ArcBallCameraOperator(ApplicationSettings* applicationSettings)
		: SceneCameraOperator(applicationSettings)
	{
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
					const auto horizontalAdjust = GetMouseXValue(static_cast<float>(event.x() - _oldCoordinates.x));
					const auto verticalAdjust = GetMouseYValue(static_cast<float>(event.y() - _oldCoordinates.y));

					_oldCoordinates.x = event.x();
					_oldCoordinates.y = event.y();

					if ((event.modifiers() & Qt::KeyboardModifier::ShiftModifier)
						|| (event.buttons() & Qt::MouseButton::ExtraButton1))
					{
						//Drag the camera across the view plane
						_parameters.TargetPosition += _camera.GetRightVector() * horizontalAdjust;
						_parameters.TargetPosition += _camera.GetUpVector() * verticalAdjust;
					}
					else
					{
						_parameters.Yaw -= horizontalAdjust;
						_parameters.Pitch -= verticalAdjust;
					}
				}
				else if (event.buttons() & Qt::MouseButton::RightButton)
				{
					const auto adjust = GetMouseYValue(static_cast<float>(event.y() - _oldCoordinates.y));

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

	void WheelEvent(QWheelEvent& event) override
	{
		if (const QPoint degrees = event.angleDelta() / 8; !degrees.isNull())
		{
			_parameters.Distance -= (degrees.y() / 15.f) * _applicationSettings->GetMouseWheelSpeed();
			UpdateArcBallCamera();
		}

		event.accept();
	}

	void CenterView(const glm::vec3& targetOrigin, const glm::vec3& cameraOrigin, float pitch, float yaw) override
	{
		SetTargetPosition(targetOrigin, pitch, yaw, glm::length(targetOrigin - cameraOrigin));
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
		_camera.SetProperties(cameraPosition, _parameters.Pitch, _parameters.Yaw, _parameters.Distance);

		emit CameraPropertiesChanged();
	}

private:
	ArcBallParameters _parameters;
	ArcBallParameters _savedParameters;
};
