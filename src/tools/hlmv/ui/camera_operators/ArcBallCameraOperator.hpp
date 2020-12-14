#pragma once

#include "ui/camera_operators/CameraOperator.hpp"

namespace ui::camera_operators
{
/**
*	@brief Maps user mouse input to a sphere that can be rotated
*/
class ArcBallCameraOperator : public CameraOperator
{
public:
	ArcBallCameraOperator() = default;
	~ArcBallCameraOperator() = default;

	void MouseEvent(const settings::GeneralSettings& generalSettings, graphics::CCamera& camera, QMouseEvent& event) override
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
					//TODO: this should be a vector, not an angle
					glm::vec3 vecViewDir = camera.GetViewDirection();

					auto horizontalAdjust = static_cast<float>(event.x() - _oldCoordinates.x);
					auto verticalAdjust = static_cast<float>(event.y() - _oldCoordinates.y);

					if (generalSettings.ShouldInvertMouseX())
					{
						horizontalAdjust = -horizontalAdjust;
					}

					if (generalSettings.ShouldInvertMouseY())
					{
						verticalAdjust = -verticalAdjust;
					}

					vecViewDir.y += horizontalAdjust;
					vecViewDir.x += verticalAdjust;

					camera.SetViewDirection(vecViewDir);

					_oldCoordinates.x = event.x();
					_oldCoordinates.y = event.y();
				}
				else if (event.buttons() & Qt::MouseButton::RightButton)
				{
					auto adjust = static_cast<float>(event.y() - _oldCoordinates.y);

					if (generalSettings.ShouldInvertMouseY())
					{
						adjust = -adjust;
					}

					camera.GetOrigin().y += adjust;

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
};
}
