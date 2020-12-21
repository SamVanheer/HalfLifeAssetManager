#pragma once

#include <glm/mat4x4.hpp>
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
	static inline const glm::vec3 DefaultViewDirection{0, 0, 1};

public:
	/**
	*	@brief Initializes the origin to 0, 0, 0 and the view direction to DefaultViewDirection.
	*	@see DefaultViewDirection
	*/
	Camera()
		: _origin(0)
		, _viewDirection(DefaultViewDirection)
	{
	}

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
	}

	/**
	*	@brief Gets the view direction.
	*/
	const glm::vec3& GetViewDirection() const { return _viewDirection; }

	/**
	*	@copydoc GetViewDirection() const
	*/
	glm::vec3& GetViewDirection() { return _viewDirection; }

	/**
	*	@brief Sets the view direction.
	*/
	void SetViewDirection(const glm::vec3& viewDirection)
	{
		_viewDirection = viewDirection;
	}

	float GetFieldOfView() const { return _fov; }

	void SetFieldOfView(float value)
	{
		_fov = value;
	}

private:
	glm::vec3 _origin;
	glm::vec3 _viewDirection;

	float _fov{90.f};
};
}
