#pragma once
#include <stack>

#include "utility/mathlib.hpp"

#include "engine/shared/renderer/IRenderContext.hpp"

namespace renderer
{
/**
*	Stack of matrices.
*/
class CMatrixStack final
{
private:
	typedef std::stack<glm::mat4x4> MatStack_t;

public:
	CMatrixStack();
	CMatrixStack( const CMatrixStack& other ) = default;
	CMatrixStack& operator=( const CMatrixStack& other ) = default;
	~CMatrixStack() = default;

	const glm::mat4x4& GetMatrix( const MatrixMode::MatrixMode mode ) const;

	void PushMatrix( const MatrixMode::MatrixMode mode );

	void PopMatrix( const MatrixMode::MatrixMode mode );

	void LoadIdentity( const MatrixMode::MatrixMode mode );

	void LoadMatrix( const MatrixMode::MatrixMode mode, const glm::mat4x4& mat );

	void LoadTransposeMatrix( const MatrixMode::MatrixMode mode, const glm::mat4x4& mat );

	void MultMatrix( const MatrixMode::MatrixMode mode, const glm::mat4x4& mat );

	void MultTransposeMatrix( const MatrixMode::MatrixMode mode, const glm::mat4x4& mat );

private:
	MatStack_t m_Stacks[ MatrixMode::COUNT ];
};
}
