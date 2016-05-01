#ifndef GRAPHICS_CCAMERA_H
#define GRAPHICS_CCAMERA_H

#include <glm/vec3.hpp>

namespace graphics
{
/**
*	Represents a camera in the world.
*/
class CCamera
{
public:
	/**
	*	The default view direction. Points to 0, 0, 1.
	*/
	static const glm::vec3 DEFAULT_VIEW_DIRECTION;

public:
	/**
	*	Default constructor. Initializes the origin to 0, 0, 0 and the view direction to DEFAULT_VIEW_DIRECTION.
	*	@see DEFAULT_VIEW_DIRECTION
	*/
	CCamera();

	/**
	*	Copy constructor.
	*	@param other Object to copy from.
	*/
	CCamera( const CCamera& other ) = default;

	/**
	*	Assignment operator.
	*	@param other Object to copy from.
	*	@return *this.
	*/
	CCamera& operator=( const CCamera& other ) = default;

	/**
	*	Constructor.
	*	@param vecOrigin Camera origin.
	*	@param vecViewDir Camera view direction.
	*/
	CCamera( const glm::vec3& vecOrigin, const glm::vec3& vecViewDir );

	/**
	*	Gets the camera origin.
	*	@return Camera origin.
	*/
	const glm::vec3& GetOrigin() const { return m_vecOrigin; }

	/**
	*	@copydoc GetOrigin() const
	*/
	glm::vec3& GetOrigin() { return m_vecOrigin; }

	/**
	*	Sets the camera origin.
	*	@param vecOrigin Origin to set.
	*/
	void SetOrigin( const glm::vec3& vecOrigin )
	{
		m_vecOrigin = vecOrigin;
	}

	/**
	*	Gets the view direction.
	*	@return View direction.
	*/
	const glm::vec3& GetViewDirection() const { return m_vecViewDir; }

	/**
	*	@copydoc GetViewDirection() const
	*/
	glm::vec3& GetViewDirection() { return m_vecViewDir; }

	/**
	*	Sets the view direction.
	*	@param vecViewDir View direction to set.
	*/
	void SetViewDirection( const glm::vec3& vecViewDir )
	{
		m_vecViewDir = vecViewDir;
	}

private:
	glm::vec3 m_vecOrigin;
	glm::vec3 m_vecViewDir;
};
}

#endif //GRAPHICS_CCAMERA_H