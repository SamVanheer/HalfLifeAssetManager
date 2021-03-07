#pragma once

#include <glm/geometric.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

#include "utility/mathlib.hpp"

namespace graphics
{
/**
*	@brief Represents a camera in the world.
*/
class Camera
{
public:
	/**
	*	@brief Initializes the origin to 0, 0, 0 and the view direction to math::ForwardVector.
	*	@see math::ForwardVector
	*/
	Camera() = default;
	Camera(const Camera&) = default;
	Camera& operator=(const Camera& other) = default;

	/**
	*	@brief Gets the camera origin.
	*/
	const glm::vec3& GetOrigin() const { return _origin; }

	/**
	*	@copydoc GetOrigin() const
	*/
	glm::vec3& GetOrigin() { return _origin; }

	/**
	*	@brief Sets the camera origin.
	*/
	void SetOrigin(const glm::vec3& origin)
	{
		_origin = origin;

		UpdateModelMatrix();
	}

	float GetPitch() const { return _pitch; }

	void SetPitch(float pitch)
	{
		SetAngles(pitch, _yaw);
	}

	float GetYaw() const { return _yaw; }

	void SetYaw(float yaw)
	{
		SetAngles(_pitch, yaw);
	}

	void SetAngles(float pitch, float yaw)
	{
		_pitch = pitch;
		_yaw = yaw;

		UpdateModelMatrix();
	}

	void SetProperties(const glm::vec3& origin, float pitch, float yaw)
	{
		_origin = origin;
		_pitch = pitch;
		_yaw = yaw;

		UpdateModelMatrix();
	}

	const glm::vec3 GetForwardVector() const { return glm::normalize(glm::vec3(_modelMatrix[0])); }

	const glm::vec3 GetRightVector() const { return glm::normalize(glm::vec3(_modelMatrix[1])); }

	const glm::vec3 GetUpVector() const { return glm::normalize(glm::vec3(_modelMatrix[2])); }

	const glm::mat4x4& GetModelMatrix() const { return _modelMatrix; }

	float GetFieldOfView() const { return _fov; }

	void SetFieldOfView(float value)
	{
		_fov = value;
	}

private:
	void UpdateModelMatrix();

private:
	glm::vec3 _origin{0};

	float _pitch{0};
	float _yaw{0};

	glm::mat4x4 _modelMatrix;

	float _fov{90.f};
};
}
