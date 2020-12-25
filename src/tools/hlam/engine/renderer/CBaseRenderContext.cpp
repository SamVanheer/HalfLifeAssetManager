#include <glm/gtc/matrix_transform.hpp>

#include "CBaseRenderContext.hpp"

namespace renderer
{
const glm::mat4x4& CBaseRenderContext::GetMatrix() const
{
	return m_MatrixStack.GetMatrix( m_MatrixMode );
}

void CBaseRenderContext::PushMatrix()
{
	m_MatrixStack.PushMatrix( m_MatrixMode );
}

void CBaseRenderContext::PopMatrix()
{
	m_MatrixStack.PopMatrix( m_MatrixMode );
}

void CBaseRenderContext::LoadIdentity()
{
	m_MatrixStack.LoadIdentity( m_MatrixMode );
}

void CBaseRenderContext::LoadMatrix( const glm::mat4x4& mat )
{
	m_MatrixStack.LoadMatrix( m_MatrixMode, mat );
}

void CBaseRenderContext::LoadTransposeMatrix( const glm::mat4x4& mat )
{
	m_MatrixStack.LoadTransposeMatrix( m_MatrixMode, mat );
}

void CBaseRenderContext::MultMatrix( const glm::mat4x4& mat )
{
	m_MatrixStack.MultMatrix( m_MatrixMode, mat );
}

void CBaseRenderContext::MultTransposeMatrix( const glm::mat4x4& mat )
{
	m_MatrixStack.MultTransposeMatrix( m_MatrixMode, mat );
}

void CBaseRenderContext::Ortho( float flLeft, float flRight, float flBottom, float flTop, float flNear, float flFar )
{
	MultMatrix( glm::ortho( flLeft, flRight, flBottom, flTop, flNear, flFar ) );
}
}