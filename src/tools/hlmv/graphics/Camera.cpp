#include "graphics/Camera.hpp"

namespace graphics
{
const glm::vec3 Camera::DEFAULT_VIEW_DIRECTION(0, 0, 1);

Camera::Camera()
	: _vecOrigin()
	, _vecViewDir(DEFAULT_VIEW_DIRECTION)
{
}
}