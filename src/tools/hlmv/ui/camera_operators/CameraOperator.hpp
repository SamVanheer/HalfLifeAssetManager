#pragma once

#include <QMouseEvent>

#include <glm/vec2.hpp>

#include "graphics/Camera.hpp"

namespace ui::camera_operators
{
class CameraOperator
{
public:
	CameraOperator(const graphics::Camera& camera)
		: _camera(camera)
	{
	}

	virtual ~CameraOperator() = 0 {}

	graphics::Camera* GetCamera() { return &_camera; }

	virtual void MouseEvent(QMouseEvent& event) = 0;

protected:
	glm::vec2 _oldCoordinates{0.f};
	Qt::MouseButtons _trackedButtons{};
	graphics::Camera _camera;
};
}
