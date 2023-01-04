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
						_targetPosition += _camera.GetRightVector() * horizontalAdjust;
						_targetPosition += _camera.GetUpVector() * verticalAdjust;
					}
					else
					{
						_yaw -= horizontalAdjust;
						_pitch -= verticalAdjust;
					}
				}
				else if (event.buttons() & Qt::MouseButton::RightButton)
				{
					const auto adjust = GetMouseYValue(static_cast<float>(event.y() - _oldCoordinates.y));

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

	void WheelEvent(QWheelEvent& event) override
	{
		if (const QPoint degrees = event.angleDelta() / 8; !degrees.isNull())
		{
			_distance -= (degrees.y() / 15.f) * _applicationSettings->GetMouseWheelSpeed();
			UpdateArcBallCamera();
		}

		event.accept();
	}

	void CenterView(const glm::vec3& targetOrigin, const glm::vec3& cameraOrigin, float pitch, float yaw) override
	{
		SetTargetPosition(targetOrigin, pitch, yaw, glm::length(targetOrigin - cameraOrigin));
	}

	bool SaveView(StateSnapshot* snapshot) override
	{
		snapshot->SetValue("TargetPosition/X", _targetPosition.x);
		snapshot->SetValue("TargetPosition/Y", _targetPosition.y);
		snapshot->SetValue("TargetPosition/Z", _targetPosition.z);

		snapshot->SetValue("Pitch", _pitch);
		snapshot->SetValue("Yaw", _yaw);
		snapshot->SetValue("Distance", _distance);

		return true;
	}

	void RestoreView(StateSnapshot* snapshot) override
	{
		 _targetPosition.x = snapshot->Value("TargetPosition/X", 0.f).toFloat();
		 _targetPosition.y = snapshot->Value("TargetPosition/Y", 0.f).toFloat();
		 _targetPosition.z = snapshot->Value("TargetPosition/Z", 0.f).toFloat();

		 _pitch = snapshot->Value("Pitch", 0.f).toFloat();
		 _yaw = snapshot->Value("Yaw", 0.f).toFloat();
		 _distance = snapshot->Value("Distance", 0.f).toFloat();

		UpdateArcBallCamera();
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
		_camera.SetProperties(cameraPosition, _pitch, _yaw, _distance);

		emit CameraPropertiesChanged();
	}

private:
	glm::vec3 _targetPosition{0};

	float _pitch{0};
	float _yaw{0};
	float _distance{0};
};
