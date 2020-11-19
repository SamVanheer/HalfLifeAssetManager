#pragma once

#include <QMouseEvent>

#include <glm/vec2.hpp>

#include "graphics/CCamera.h"

namespace ui::camera_operators
{
class CameraOperator
{
public:
	virtual ~CameraOperator() = 0 {}

	virtual void MouseEvent(graphics::CCamera& camera, QMouseEvent& event) = 0;

protected:
	glm::vec2 _oldCoordinates{0.f};
	Qt::MouseButtons _trackedButtons{};
};
}
