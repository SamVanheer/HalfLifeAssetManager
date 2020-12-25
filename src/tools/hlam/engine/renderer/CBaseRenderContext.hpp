#pragma once

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

	const glm::mat4x4& GetMatrix() const override;

	void PushMatrix() override;

	void PopMatrix() override;

	void LoadIdentity() override;

	void LoadMatrix( const glm::mat4x4& mat ) override;

	void LoadTransposeMatrix( const glm::mat4x4& mat ) override;

	void MultMatrix( const glm::mat4x4& mat ) override;

	void MultTransposeMatrix( const glm::mat4x4& mat ) override;

	void Ortho( float flLeft, float flRight, float flBottom, float flTop, float flNear, float flFar ) override;

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
