#include "CCamera.h"

namespace graphics
{
const glm::vec3 CCamera::DEFAULT_VIEW_DIRECTION( 0, 0, 1 );

CCamera::CCamera()
	: m_vecOrigin()
	, m_vecViewDir( DEFAULT_VIEW_DIRECTION )
{
}

CCamera::CCamera( const glm::vec3& vecOrigin, const glm::vec3& vecViewDir )
	: m_vecOrigin( vecOrigin )
	, m_vecViewDir( vecViewDir )
{
}
}