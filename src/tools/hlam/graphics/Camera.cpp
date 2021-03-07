#include <glm/trigonometric.hpp>
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

	UpdateViewMatrix();
}

void Camera::UpdateViewMatrix()
{
	_viewMatrix = glm::lookAt(GetOrigin(), GetOrigin() + GetForwardVector(), GetUpVector());
}

void Camera::UpdateProjectionMatrix()
{
	//This can be called when we haven't gotten the window size yet, or the window has size 0, 0 for whatever reason
	const float aspectRatio = (_windowWidth != 0 && _windowHeight != 0) ? _windowWidth / _windowHeight : 1;

	_projectionMatrix = glm::perspective(glm::radians(GetFieldOfView()), aspectRatio, 1.0f, static_cast<float>(1 << 24));
}
}
