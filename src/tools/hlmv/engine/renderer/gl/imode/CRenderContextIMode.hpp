#ifndef ENGINE_RENDERER_GL_IMODE_CRENDERCONTEXTIMODE_H
#define ENGINE_RENDERER_GL_IMODE_CRENDERCONTEXTIMODE_H

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

	void LoadMatrix( const Mat4x4& mat ) override;

	void LoadTransposeMatrix( const Mat4x4& mat ) override;

	void MultMatrix( const Mat4x4& mat ) override;

	void MultTransposeMatrix( const Mat4x4& mat ) override;
};
}

#endif //ENGINE_RENDERER_GL_IMODE_CRENDERCONTEXTIMODE_H