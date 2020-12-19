#include <glm/gtc/matrix_transform.hpp>

#include "core/shared/Logging.hpp"

#include "CBaseGLRenderContext.hpp"

static_assert( sizeof( renderer::HTexture_t ) == sizeof( GLuint ), "Unsupported handle size!" );

namespace renderer
{
GLenum ImageFormatToGL( const ImageFormat format )
{
	switch( format )
	{
	case ImageFormat::RGB:				return GL_RGB;

	case ImageFormat::RGBA:				return GL_RGBA;

	case ImageFormat::ALPHA:			return GL_ALPHA;

	case ImageFormat::LUMINANCE:		return GL_LUMINANCE;

	case ImageFormat::LUMINANCE_ALPHA:	return GL_LUMINANCE_ALPHA;

	default:
		{
			Error( "ImageFormatToGL: Invalid image format \"%d\"\n", format );
			return GL_RGB;
		}
	}
}

GLenum MinFilterToGL( const MinFilter min )
{
	switch( min )
	{
	case MinFilter::NEAREST:					return GL_NEAREST;
	case MinFilter::LINEAR:						return GL_LINEAR;
	case MinFilter::NEAREST_MIPMAP_NEAREST:		return GL_NEAREST_MIPMAP_NEAREST;
	case MinFilter::NEAREST_MIPMAP_LINEAR:		return GL_NEAREST_MIPMAP_LINEAR;
	case MinFilter::LINEAR_MIPMAP_NEAREST:		return GL_LINEAR_MIPMAP_NEAREST;
	case MinFilter::LINEAR_MIPMAP_LINEAR:		return GL_LINEAR_MIPMAP_LINEAR;

	default:
		{
			Error( "MinFilterToGL: Invalid min filter \"%d\"\n", min );
			return GL_NEAREST;
		}
	}
}

GLenum MagFilterToGL( const MagFilter mag )
{
	switch( mag )
	{
	case MagFilter::NEAREST:	return GL_NEAREST;
	case MagFilter::LINEAR:		return GL_LINEAR;

	default:
		{
			Error( "MagFilterToGL: Invalid mag filter \"%d\"\n", mag );
			return GL_NEAREST;
		}
	}
}

void CBaseGLRenderContext::Viewport( int iX, int iY, int iWidth, int iHeight )
{
	glViewport( iX, iY, iWidth, iHeight );
}

void CBaseGLRenderContext::PerspectiveY( vec_t flFOVY, vec_t flAspect, vec_t flNear, vec_t flFar )
{
	MultMatrix( glm::perspective( flFOVY, flAspect, flNear, flFar ) );
}

void CBaseGLRenderContext::ClearColor( const Color32& color )
{
	glClearColor( color.r, color.g, color.b, color.a );
}

void CBaseGLRenderContext::ClearColor( const Color24& color, float flA  )
{
	glClearColor( color.r, color.g, color.b, flA );
}

void CBaseGLRenderContext::ClearColor( float flR, float flG, float flB, float flA )
{
	glClearColor( flR, flG, flB, flA );
}

void CBaseGLRenderContext::Clear( const ClearBits_t bits )
{
	GLbitfield mask = 0;

	if( bits & ClearBit::COLOR )
		mask |= GL_COLOR_BUFFER_BIT;

	if( bits & ClearBit::DEPTH )
		mask |= GL_DEPTH_BUFFER_BIT;

	if( bits & ClearBit::STENCIL )
		mask |= GL_STENCIL_BUFFER_BIT;

	glClear( mask );
}

void CBaseGLRenderContext::SetCullFace( const CullFace cullFace )
{
	GLenum mode;

	switch( cullFace )
	{
	case CullFace::BACK:
		{
			mode = GL_BACK;
			break;
		}

	case CullFace::FRONT:
		{
			mode = GL_FRONT;
			break;
		}

	case CullFace::FRONT_AND_BACK:
		{
			mode = GL_FRONT_AND_BACK;
			break;
		}

	default:
		{
			Error( "CBaseGLRenderContext::SetCullFace: Invalid mode \"%d\" specified!\n", cullFace );
			return;
		}
	}

	glCullFace( mode );
}

ReadBuffer CBaseGLRenderContext::GetReadBuffer() const
{
	GLint oldReadBuffer;

	glGetIntegerv( GL_READ_BUFFER, &oldReadBuffer );

	return oldReadBuffer == GL_FRONT ? ReadBuffer::FRONT : ReadBuffer::BACK;
}

void CBaseGLRenderContext::SetReadBuffer( const ReadBuffer buffer )
{
	glReadBuffer( buffer == ReadBuffer::FRONT ? GL_FRONT : GL_BACK );
}

bool CBaseGLRenderContext::ReadPixels( int iX, int iY, int iWidth, int iHeight, const ImageFormat format, byte* pOutBuffer )
{
	GLenum imageFormat = ImageFormatToGL( format );

	glReadPixels( iX, iY, iWidth, iHeight, imageFormat, GL_UNSIGNED_BYTE, pOutBuffer );

	return true;
}

HTexture_t CBaseGLRenderContext::CreateTexture( const int mipmaps, const ImageFormat format, const int iWidth, const int iHeight, const byte* pData )
{
	GLenum imageFormat = ImageFormatToGL( format );

	GLuint texture;

	glGenTextures( 1, &texture );

	glBindTexture( GL_TEXTURE_2D, texture );

	//Set pack alignment to 1 so images will load correctly if rows are not multiple of 4
	GLint oldUnpackAlignment;

	glGetIntegerv(GL_PACK_ALIGNMENT, &oldUnpackAlignment);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glTexImage2D( GL_TEXTURE_2D, mipmaps, imageFormat, iWidth, iHeight, 0, imageFormat, GL_UNSIGNED_BYTE, pData );

	glPixelStorei(GL_UNPACK_ALIGNMENT, oldUnpackAlignment);

	//TODO: error handling.

	//TODO: make this customizable?
	//TODO: wrong target
	glTexEnvi( GL_TEXTURE_2D, GL_TEXTURE_ENV_MODE, GL_MODULATE );

	return GLToTexHandle( texture );
}

void CBaseGLRenderContext::DestroyTexture( HTexture_t hTexture )
{
	if( hTexture == NULL_TEXTURE_HANDLE )
		return;

	GLuint tex = TexHandleToGL( hTexture );

	glDeleteTextures( 1, &tex );
}

void CBaseGLRenderContext::BindTexture( HTexture_t hTexture )
{
	glBindTexture( GL_TEXTURE_2D, TexHandleToGL( hTexture ) );
}

void CBaseGLRenderContext::SetMinMagFilters( const MinFilter min, const MagFilter mag )
{
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, MinFilterToGL( min ) );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, MagFilterToGL( mag ) );
}
}