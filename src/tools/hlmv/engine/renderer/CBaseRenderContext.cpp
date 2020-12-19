#include <glm/gtc/matrix_transform.hpp>

#include "CBaseRenderContext.hpp"

namespace renderer
{
const Mat4x4& CBaseRenderContext::GetMatrix() const
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

void CBaseRenderContext::LoadMatrix( const Mat4x4& mat )
{
	m_MatrixStack.LoadMatrix( m_MatrixMode, mat );
}

void CBaseRenderContext::LoadTransposeMatrix( const Mat4x4& mat )
{
	m_MatrixStack.LoadTransposeMatrix( m_MatrixMode, mat );
}

void CBaseRenderContext::MultMatrix( const Mat4x4& mat )
{
	m_MatrixStack.MultMatrix( m_MatrixMode, mat );
}

void CBaseRenderContext::MultTransposeMatrix( const Mat4x4& mat )
{
	m_MatrixStack.MultTransposeMatrix( m_MatrixMode, mat );
}

void CBaseRenderContext::Ortho( vec_t flLeft, vec_t flRight, vec_t flBottom, vec_t flTop, vec_t flNear, vec_t flFar )
{
	MultMatrix( glm::ortho( flLeft, flRight, flBottom, flTop, flNear, flFar ) );
}
}