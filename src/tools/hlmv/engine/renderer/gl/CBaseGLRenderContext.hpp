#pragma once

#include "engine/renderer/CBaseRenderContext.hpp"

#include "graphics/OpenGL.hpp"

namespace renderer
{
GLenum ImageFormatToGL( const ImageFormat format );

inline HTexture_t GLToTexHandle( const GLuint texture )
{
	return reinterpret_cast<HTexture_t>( texture );
}

inline GLuint TexHandleToGL( const HTexture_t hTexture )
{
	return reinterpret_cast<GLuint>( hTexture );
}

GLenum MinFilterToGL( const MinFilter min );

GLenum MagFilterToGL( const MagFilter mag );

class CBaseGLRenderContext : public CBaseRenderContext
{
public:

	void Viewport( int iX, int iY, int iWidth, int iHeight ) override;

	void PerspectiveY( vec_t flFOVY, vec_t flAspect, vec_t flNear, vec_t flFar ) override;

	void ClearColor( const Color32& color ) override;

	void ClearColor( const Color24& color, float flA = 0 ) override;

	void ClearColor( float flR = 0, float flG = 0, float flB = 0, float flA = 0 ) override;

	void Clear( const ClearBits_t bits ) override;

	void SetCullFace( const CullFace cullFace ) override;

	ReadBuffer GetReadBuffer() const override;

	void SetReadBuffer( const ReadBuffer buffer ) override;

	bool ReadPixels( int iX, int iY, int iWidth, int iHeight, const ImageFormat format, byte* pOutBuffer ) override;

	HTexture_t CreateTexture( const int mipmaps, const ImageFormat format, const int iWidth, const int iHeight, const byte* pData ) override;

	void DestroyTexture( HTexture_t hTexture ) override;

	void BindTexture( HTexture_t hTexture ) override;

	void SetMinMagFilters( const MinFilter min, const MagFilter mag ) override;

private:
};
}
