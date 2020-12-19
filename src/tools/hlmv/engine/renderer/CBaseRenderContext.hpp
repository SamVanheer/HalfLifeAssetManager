#ifndef ENGINE_RENDERER_CBASERENDERCONTEXT_H
#define ENGINE_RENDERER_CBASERENDERCONTEXT_H

#include "engine/shared/renderer/IRenderContext.hpp"

#include "engine/renderer/util/CMatrixStack.hpp"

namespace renderer
{
/**
*	Base class for render contexts. Defines shared operations.
*/
class CBaseRenderContext : public IRenderContext
{
public:

	//Matrix operations
	MatrixMode::MatrixMode GetMatrixMode() const override { return m_MatrixMode; }

	void MatrixMode( const MatrixMode::MatrixMode mode ) override { m_MatrixMode = mode; }

	const Mat4x4& GetMatrix() const override;

	void PushMatrix() override;

	void PopMatrix() override;

	void LoadIdentity() override;

	void LoadMatrix( const Mat4x4& mat ) override;

	void LoadTransposeMatrix( const Mat4x4& mat ) override;

	void MultMatrix( const Mat4x4& mat ) override;

	void MultTransposeMatrix( const Mat4x4& mat ) override;

	void Ortho( vec_t flLeft, vec_t flRight, vec_t flBottom, vec_t flTop, vec_t flNear, vec_t flFar ) override;

protected:
	/**
	*	Gets the matrix stack for use by subclasses.
	*	@return The matrix stack.
	*/
	const CMatrixStack& GetMatrixStack() const { return m_MatrixStack; }

	/**
	*	@copydoc GetMatrixStack() const
	*/
	CMatrixStack& GetMatrixStack() { return m_MatrixStack; }

private:
	MatrixMode::MatrixMode m_MatrixMode = MatrixMode::MODEL;

	//Needed for all renderers because our API separates the Model and View matrices.
	//We need to keep track of them separately so mult operations work in graphics APIs that combine matrices.
	CMatrixStack m_MatrixStack;
};
}

#endif //ENGINE_RENDERER_CBASERENDERCONTEXT_H