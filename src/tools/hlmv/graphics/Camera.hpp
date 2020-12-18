#pragma once

#include <glm/vec3.hpp>

namespace graphics
{
/**
*	@brief Represents a camera in the world.
*/
class Camera
{
public:
	/**
	*	@brief The default view direction. Points to 0, 0, 1.
	*/
	static const glm::vec3 DEFAULT_VIEW_DIRECTION;

public:
	/**
	*	Initializes the origin to 0, 0, 0 and the view direction to DEFAULT_VIEW_DIRECTION.
	*	@see DEFAULT_VIEW_DIRECTION
	*/
	Camera();

	Camera(const Camera&) = default;
	Camera& operator=(const Camera& other) = default;

	/**
	*	@brief Gets the camera origin.
	*/
	const glm::vec3& GetOrigin() const { return _vecOrigin; }

	/**
	*	@copydoc GetOrigin() const
	*/
	glm::vec3& GetOrigin() { return _vecOrigin; }

	/**
	*	@brief Sets the camera origin.
	*/
	void SetOrigin(const glm::vec3& vecOrigin)
	{
		_vecOrigin = vecOrigin;
	}

	/**
	*	@brief Gets the view direction.
	*/
	const glm::vec3& GetViewDirection() const { return _vecViewDir; }

	/**
	*	@copydoc GetViewDirection() const
	*/
	glm::vec3& GetViewDirection() { return _vecViewDir; }

	/**
	*	@brief Sets the view direction.
	*/
	void SetViewDirection(const glm::vec3& vecViewDir)
	{
		_vecViewDir = vecViewDir;
	}

	float GetFieldOfView() const { return _fov; }

	void SetFieldOfView(float value)
	{
		_fov = value;
	}

private:
	glm::vec3 _vecOrigin;
	glm::vec3 _vecViewDir;

	float _fov{90.f};
};
}
