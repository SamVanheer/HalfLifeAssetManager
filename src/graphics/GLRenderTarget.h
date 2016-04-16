#ifndef GRAPHICS_GLRENDERTARGET_H
#define GRAPHICS_GLRENDERTARGET_H

#include <memory>

#include "OpenGL.h"

/**
*	This class represents a render target. It can be used to draw to and then retrieve the pixel contents.
*	You must set the context to current yourself.
*/
class GLRenderTarget final
{
public:
	/**
	*	Constructs a new render target. If bCreate is true, initializes its GL data.
	*/
	GLRenderTarget( const bool bCreate );
	~GLRenderTarget();

	/**
	*	Gets the framebuffer ID.
	*/
	GLuint GetFrameBuffer() const { return m_FrameBuffer; }

	/**
	*	Gets the texture ID.
	*/
	GLuint GetTexture() const { return m_Texture; }

	/**
	*	Returns whether this render target exists.
	*/
	bool Exists() const;

	/**
	*	Creates the render target if it doesn't exist yet.
	*	@param true on success, false otherwise.
	*/
	bool Create();

	/**
	*	Destroys the render target if it exists.
	*/
	void Destroy();

	/**
	*	Sets the render target's dimensions, and sets up the depth buffer.
	*	@param iWidth Width, in pixels.
	*	@param iHeight Height, in pixels.
	*/
	void Setup( const GLsizei iWidth, const GLsizei iHeight, const bool bUseDepthBuffer );

	/**
	*	Binds the render target.
	*	@param true on success, false otherwise.
	*/
	bool Bind();

	/**
	*	Unbinds the render target.
	*/
	void Unbind();

	/**
	*	Gets the render target's status. Use glFrameBufferStatusToString to get a string representation.
	*/
	GLenum GetStatus() const;

	/**
	*	Finish any pending draw operations.
	*/
	void FinishDraw();

	/**
	*	Gets pixels from the render target.
	*/
	void GetPixels( const GLsizei iWidth, const GLsizei iHeight, const GLenum format, const GLenum type, void* pPixels );

private:
	GLuint m_FrameBuffer	= 0;
	GLuint m_Texture		= GL_INVALID_TEXTURE_ID;
	GLuint m_DepthBuffer	= 0;

private:
	GLRenderTarget( const GLRenderTarget& ) = delete;
	GLRenderTarget& operator=( const GLRenderTarget& ) = delete;
};

#endif //GRAPHICS_GLRENDERTARGET_H