#pragma once

#include <glm/geometric.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "utility/mathlib.hpp"

class QMouseEvent;
class QWheelEvent;

namespace graphics
{
enum class ProjectionMode
{
	Perspective = 0,
	Orthographic
};

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

	void SetProperties(const glm::vec3& origin, float pitch, float yaw, float distance)
	{
		_origin = origin;
		_pitch = pitch;
		_yaw = yaw;
		_distance = distance;

		UpdateModelMatrix();
	}

	float GetFieldOfView() const { return _fov; }

	void SetFieldOfView(float value)
	{
		_fov = value;

		UpdateProjectionMatrix();
	}

	float GetDistance() const { return _distance; }

	void SetDistance(float distance)
	{
		_distance = distance;
		UpdateProjectionMatrix();
	}

	ProjectionMode GetProjectionMode() const { return _projectionMode; }

	void SetProjectionMode(ProjectionMode projectionMode)
	{
		if (_projectionMode != projectionMode)
		{
			_projectionMode = projectionMode;
			UpdateProjectionMatrix();
		}
	}

	void SetWindowSize(float windowWidth, float windowHeight)
	{
		_windowWidth = windowWidth;
		_windowHeight = windowHeight;

		UpdateProjectionMatrix();
	}

	const glm::vec3 GetForwardVector() const { return glm::normalize(glm::vec3(_modelMatrix[0])); }

	const glm::vec3 GetRightVector() const { return glm::normalize(glm::vec3(_modelMatrix[1])); }

	const glm::vec3 GetUpVector() const { return glm::normalize(glm::vec3(_modelMatrix[2])); }

	const glm::mat4x4& GetModelMatrix() const { return _modelMatrix; }

	const glm::mat4x4& GetViewMatrix() const { return _viewMatrix; }

	const glm::mat4x4& GetProjectionMatrix() const { return _projectionMatrix; }

private:
	void UpdateModelMatrix();
	void UpdateViewMatrix();
	void UpdateProjectionMatrix();

private:
	glm::vec3 _origin{0};

	float _pitch{0};
	float _yaw{0};
	float _fov{90.f};
	float _distance{0};

	ProjectionMode _projectionMode{ProjectionMode::Perspective};

	float _windowWidth{0.f};
	float _windowHeight{0.f};

	glm::mat4x4 _modelMatrix{glm::identity<glm::mat4x4>()};
	glm::mat4x4 _viewMatrix{glm::identity<glm::mat4x4>()};
	glm::mat4x4 _projectionMatrix{glm::identity<glm::mat4x4>()};
};

/**
*	@brief Represents the operation of a camera.
*/
class ICameraOperator
{
public:
	virtual ~ICameraOperator() = default;

	virtual Camera* GetCamera() = 0;

	virtual void MouseEvent(QMouseEvent& event) = 0;

	virtual void WheelEvent(QWheelEvent& event) = 0;
};
}
