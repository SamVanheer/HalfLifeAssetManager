#pragma once

#include "engine/renderer/gl/CBaseGLRenderContext.hpp"

namespace renderer
{
/**
*	Immediate mode render context.
*/
class CRenderContextIMode : public CBaseGLRenderContext
{
public:
	typedef CBaseRenderContext BaseClass;

public:
	CRenderContextIMode() = default;

	void MatrixMode( const MatrixMode::MatrixMode mode ) override;

	void PushMatrix() override;

	void PopMatrix() override;

	void LoadIdentity() override;

	void LoadMatrix( const glm::mat4x4& mat ) override;

	void LoadTransposeMatrix( const glm::mat4x4& mat ) override;

	void MultMatrix( const glm::mat4x4& mat ) override;

	void MultTransposeMatrix( const glm::mat4x4& mat ) override;
};
}
