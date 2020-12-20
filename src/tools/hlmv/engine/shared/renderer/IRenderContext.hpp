#pragma once

#include "core/shared/Const.hpp"
#include "core/shared/Utility.hpp"

#include "utility/mathlib.hpp"

namespace renderer
{
namespace MatrixMode
{
/**
*	Matrix modes.
*/
enum MatrixMode
{
	FIRST		= 0,
	MODEL		= FIRST,
	VIEW,
	PROJECTION,

	//TODO: OpenGL has texture modes as well. Consider adding them.

	LAST		= PROJECTION,

	COUNT
};
}

typedef unsigned int ClearBits_t;

namespace ClearBit
{
/**
*	Bits that identify buffers that can be cleared.
*/
enum ClearBit : ClearBits_t
{
	/**
	*	Color buffer.
	*/
	COLOR = Bit( 0 ),

	/**
	*	Depth buffer.
	*/
	DEPTH = Bit( 1 ),

	/**
	*	Stencil buffer.
	*/
	STENCIL = Bit( 2 )
};
}

/**
*	Cull faces.
*/
enum class CullFace
{
	/**
	*	Cull the back face.
	*/
	BACK,

	/**
	*	Cull the front face.
	*/
	FRONT,

	/**
	*	Cull both front and back faces.
	*/
	FRONT_AND_BACK
};

/**
*	Read buffer.
*/
enum class ReadBuffer
{
	/**
	*	Back buffer.
	*/
	BACK,

	/**
	*	Front buffer.
	*/
	FRONT
};

/**
*	Image formats.
*/
enum ImageFormat
{
	RGB,
	RGBA,
	ALPHA,
	LUMINANCE,
	LUMINANCE_ALPHA
};

/**
*	Minifying filter.
*/
enum class MinFilter
{
	NEAREST,
	LINEAR,
	NEAREST_MIPMAP_NEAREST,
	LINEAR_MIPMAP_NEAREST,
	NEAREST_MIPMAP_LINEAR,
	LINEAR_MIPMAP_LINEAR
};

/**
*	Magnification filter.
*/
enum class MagFilter
{
	NEAREST,
	LINEAR
};

/**
*	Handle to a texture.
*/
typedef void* HTexture_t;

/**
*	Null texture handle. This represents "no" texture.
*/
#define NULL_TEXTURE_HANDLE ( reinterpret_cast<HTexture_t>( 0 ) )

/**
*	Renderer context. Provides access to a variety of context specific operations.
*/
class IRenderContext
{
public:
	virtual ~IRenderContext() = 0 {}

	//Matrix operations
	/**
	*	@return The current matrix mode.
	*/
	virtual MatrixMode::MatrixMode GetMatrixMode() const = 0;

	/**
	*	Sets the current matrix mode.
	*	@param mode Mode.
	*/
	virtual void MatrixMode( const MatrixMode::MatrixMode mode ) = 0;

	/**
	*	Gets the current matrix for the active matrix mode.
	*	@return Current matrix.
	*/
	virtual const Mat4x4& GetMatrix() const = 0;

	/**
	*	Pushes the current matrix on the stack.
	*/
	virtual void PushMatrix() = 0;

	/**
	*	Pops the matrix that is on the stack and sets it to the current matrix.
	*/
	virtual void PopMatrix() = 0;

	/**
	*	Loads the identity matrix.
	*/
	virtual void LoadIdentity() = 0;

	/**
	*	Loads the given matrix into the current matrix.
	*/
	virtual void LoadMatrix( const Mat4x4& mat ) = 0;

	/**
	*	Loads the given row-major matrix into the current matrix.
	*/
	virtual void LoadTransposeMatrix( const Mat4x4& mat ) = 0;

	/**
	*	Multiplies the current matrix with the given matrix.
	*/
	virtual void MultMatrix( const Mat4x4& mat ) = 0;

	/**
	*	Multiplies the current matrix with the given row-major matrix.
	*/
	virtual void MultTransposeMatrix( const Mat4x4& mat ) = 0;

	/**
	*	Multiplies the current matrix with an orthographic matrix defined by the given parameters.
	*	@param flLeft Left position on-screen.
	*	@param flRight Right position on-screen.
	*	@param flBottom Bottom position on-screen.
	*	@param flTop Top position on-screen.
	*	@param flNear Near Z clip plane.
	*	@param flFar Far Z clip plane.
	*/
	virtual void Ortho( vec_t flLeft, vec_t flRight, vec_t flBottom, vec_t flTop, vec_t flNear, vec_t flFar ) = 0;

	/**
	*	Sets the viewport.
	*	@param iX Left corner of the viewport on-screen.
	*	@param iY Bottom corner of the viewport on-screen.
	*	@param iWidth Width of the viewport.
	*	@param iHeight Height of the viewport.
	*/
	virtual void Viewport( int iX, int iY, int iWidth, int iHeight ) = 0;

	/**
	*	Multiplies the current matrix with a perspective matrix defined by the given parameters.
	*	@param flFOVY Field of view in radians, in the Y direction.
	*	@param flAspect Aspect ratio.
	*	@param flNear Near Z clip plane.
	*	@param flFar Far Z clip plane.
	*/
	virtual void PerspectiveY( vec_t flFOVY, vec_t flAspect, vec_t flNear, vec_t flFar ) = 0;

	/**
	*	Sets the clear color.
	*	@param color RGBA color.
	*/
	virtual void ClearColor( const Color32& color ) = 0;

	/**
	*	Sets the clear color.
	*	@param color RGB color.
	*	@param flA Alpha value.
	*/
	virtual void ClearColor( const Color24& color, float flA = 0 ) = 0;

	/**
	*	Sets the clear color.
	*	@param flR Red color.
	*	@param flG Green color.
	*	@param flB Blue color.
	*	@param flA Alpha value.
	*/
	virtual void ClearColor( float flR = 0, float flG = 0, float flB = 0, float flA = 0 ) = 0;

	/**
	*	Clears the given buffers.
	*	@see ClearBit
	*/
	virtual void Clear( const ClearBits_t bits ) = 0;

	/**
	*	Sets the cull face.
	*	@see CullFace
	*/
	virtual void SetCullFace( const CullFace cullFace ) = 0;

	/**
	*	@return The current read buffer setting.
	*	@see SetReadBuffer
	*/
	virtual ReadBuffer GetReadBuffer() const = 0;

	/**
	*	Sets the buffer from which ReadPixels will read.
	*	@param buffer Buffer to read from.
	*	@see ReadBuffer
	*	@see ReadPixels
	*/
	virtual void SetReadBuffer( const ReadBuffer buffer ) = 0;

	/**
	*	Reads pixels from the currently activated buffer.
	*	@param iX X position.
	*	@param iY Y position.
	*	@param iWidth Width.
	*	@param iHeight Height.
	*	@param format Image format to read.
	*	@param pOutBuffer Output buffer to place the contents of the buffer in.
	*			Must be iWidth * iHeight * channels bytes large, where channels is the number of color channels, including the alpha channel.
	*	@return true on success, false otherwise.
	*/
	virtual bool ReadPixels( int iX, int iY, int iWidth, int iHeight, const ImageFormat format, byte* pOutBuffer ) = 0;

	/**
	*	Creates a new texture.
	*	TODO texture type
	*	@param mipmaps Number of mipmaps to generate.
	*	@param format The image format that the given data is encoded in.
	*	@param iWidth Width of the texture.
	*	@param iHeight Height of the texture.
	*	@param pData Image data.
	*	@return Texture handle, or NULL_TEXTURE_HANDLE if an error occurred.
	*	@see NULL_TEXTURE_HANDLE
	*/
	virtual HTexture_t CreateTexture( const int mipmaps, const ImageFormat format, const int iWidth, const int iHeight, const byte* pData ) = 0;

	/**
	*	Destroys a given texture. The handle will be invalid after this call.
	*/
	virtual void DestroyTexture( HTexture_t hTexture ) = 0;

	/**
	*	Binds the given texture. Can be NULL_TEXTURE_HANDLE, in which case the current texture is unbound.
	*	TODO texture type
	*	@see NULL_TEXTURE_HANDLE
	*/
	virtual void BindTexture( HTexture_t hTexture ) = 0;

	/**
	*	Sets the minmag filters for the current texture.
	*	@param min Minifying filter.
	*	@param mag Magnification filter.
	*/
	virtual void SetMinMagFilters( const MinFilter min, const MagFilter mag ) = 0;
};
}
