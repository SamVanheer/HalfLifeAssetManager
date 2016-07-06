#ifndef ENGINE_RENDERER_UTIL_CMATRIXSTACK_H
#define ENGINE_RENDERER_UTIL_CMATRIXSTACK_H

#include <stack>

#include "utility/mathlib.h"

#include "engine/shared/renderer/IRenderContext.h"

namespace renderer
{
/**
*	Stack of matrices.
*/
class CMatrixStack final
{
private:
	typedef std::stack<Mat4x4> MatStack_t;

public:
	CMatrixStack();
	CMatrixStack( const CMatrixStack& other ) = default;
	CMatrixStack& operator=( const CMatrixStack& other ) = default;
	~CMatrixStack() = default;

	const Mat4x4& GetMatrix( const MatrixMode::MatrixMode mode ) const;

	void PushMatrix( const MatrixMode::MatrixMode mode );

	void PopMatrix( const MatrixMode::MatrixMode mode );

	void LoadIdentity( const MatrixMode::MatrixMode mode );

	void LoadMatrix( const MatrixMode::MatrixMode mode, const Mat4x4& mat );

	void LoadTransposeMatrix( const MatrixMode::MatrixMode mode, const Mat4x4& mat );

	void MultMatrix( const MatrixMode::MatrixMode mode, const Mat4x4& mat );

	void MultTransposeMatrix( const MatrixMode::MatrixMode mode, const Mat4x4& mat );

private:
	MatStack_t m_Stacks[ MatrixMode::COUNT ];
};
}

#endif //ENGINE_RENDERER_UTIL_CMATRIXSTACK_H