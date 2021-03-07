#include <glm/trigonometric.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include "graphics/Camera.hpp"

namespace graphics
{
void Camera::UpdateModelMatrix()
{
	_modelMatrix = glm::identity<glm::mat4x4>();

	_modelMatrix = glm::translate(_modelMatrix, _origin);

	const glm::vec3 angles{FixAngles({_pitch, _yaw, 0})};

	const glm::quat rotation{glm::vec3{glm::radians(angles.z), glm::radians(-angles.x), glm::radians(angles.y)}};

	_modelMatrix *= glm::mat4_cast(rotation);
}
}
