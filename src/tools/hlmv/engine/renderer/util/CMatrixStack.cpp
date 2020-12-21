#include <glm/gtc/matrix_transform.hpp>

#include "CMatrixStack.hpp"

namespace renderer
{
CMatrixStack::CMatrixStack()
{
	//Push identity matrices for each stack.
	for( auto& stack : m_Stacks )
	{
		stack.push(glm::mat4x4() );
	}
}

const glm::mat4x4& CMatrixStack::GetMatrix( const MatrixMode::MatrixMode mode ) const
{
	return m_Stacks[ mode ].top();
}

void CMatrixStack::PushMatrix( const MatrixMode::MatrixMode mode )
{
	m_Stacks[ mode ].push(glm::mat4x4() );
}

void CMatrixStack::PopMatrix( const MatrixMode::MatrixMode mode )
{
	m_Stacks[ mode ].pop();
}

void CMatrixStack::LoadIdentity( const MatrixMode::MatrixMode mode )
{
	m_Stacks[ mode ].top() = glm::mat4x4();
}

void CMatrixStack::LoadMatrix( const MatrixMode::MatrixMode mode, const glm::mat4x4& mat )
{
	m_Stacks[ mode ].top() = mat;
}

void CMatrixStack::LoadTransposeMatrix( const MatrixMode::MatrixMode mode, const glm::mat4x4& mat )
{
	m_Stacks[ mode ].top() = glm::transpose( mat );
}

void CMatrixStack::MultMatrix( const MatrixMode::MatrixMode mode, const glm::mat4x4& mat )
{
	m_Stacks[ mode ].top() *= mat;
}

void CMatrixStack::MultTransposeMatrix( const MatrixMode::MatrixMode mode, const glm::mat4x4& mat )
{
	m_Stacks[ mode ].top() *= glm::transpose( mat );
}
}