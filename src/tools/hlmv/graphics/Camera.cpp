#include <glm/trigonometric.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include "graphics/Camera.hpp"

namespace graphics
{
void Camera::UpdateViewMatrix()
{
	_viewMatrix = glm::identity<glm::mat4x4>();

	_viewMatrix = glm::translate(_viewMatrix, _origin);

	const glm::vec3 angles{FixAngles({_pitch, _yaw, 0})};

	const glm::quat rotation{glm::vec3{glm::radians(angles.z), glm::radians(-angles.x), glm::radians(angles.y)}};

	_viewMatrix *= glm::mat4_cast(rotation);
}
}
