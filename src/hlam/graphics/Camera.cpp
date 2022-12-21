#include <algorithm>

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

	if (_projectionMode == ProjectionMode::Orthographic)
	{
		UpdateProjectionMatrix();
	}
}

void Camera::UpdateProjectionMatrix()
{
	//This can be called when we haven't gotten the window size yet, or the window has size 0, 0 for whatever reason
	float width = _windowWidth != 0 ? _windowWidth : 1;
	float height = _windowHeight != 0 ? _windowHeight : 1;

	switch (_projectionMode)
	{
	case ProjectionMode::Perspective:
		_projectionMatrix = glm::perspective(
			glm::radians(GetFieldOfView()), width / height, 1.0f, static_cast<float>(1 << 24));
		break;

	case ProjectionMode::Orthographic:
	{
		// Adjust the distance based on the average distance from window center to its edges
		// to produce a zoom roughly equivalent to perspective distance.
		const float ratio = std::sqrt((width / 2) * (height / 2));
		const float zoom = std::max(0.f, _distance / ratio);

		width *= zoom;
		height *= zoom;

		const float halfWidth = width / 2;
		const float halfHeight = height / 2;

		// Near Z has to be behind the camera to prevent models from clipping regardless.
		_projectionMatrix = glm::ortho(
			-halfWidth, halfWidth, -halfHeight, halfHeight, -100.0f, static_cast<float>(1 << 12));
		break;
	}
	}
}
}
