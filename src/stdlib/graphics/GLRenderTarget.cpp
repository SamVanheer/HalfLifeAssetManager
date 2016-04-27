#include "GLRenderTarget.h"

GLRenderTarget::GLRenderTarget( const bool bCreate )
{
	glGenFramebuffers( 1, &m_FrameBuffer );

	glBindFramebuffer( GL_FRAMEBUFFER, m_FrameBuffer );

	glGenTextures( 1, &m_Texture );

	if( bCreate )
		Create();
}

GLRenderTarget::~GLRenderTarget()
{
	Destroy();
}

bool GLRenderTarget::Exists() const
{
	return m_FrameBuffer != 0 && m_Texture != GL_INVALID_TEXTURE_ID;
}

bool GLRenderTarget::Create()
{
	if( Exists() )
		return true;

	glGenFramebuffers( 1, &m_FrameBuffer );

	glGenTextures( 1, &m_Texture );

	glGenRenderbuffers( 1, &m_DepthBuffer );

	return m_FrameBuffer != 0;
}

void GLRenderTarget::Destroy()
{
	if( m_DepthBuffer != 0 )
	{
		glDeleteRenderbuffers( 1, &m_DepthBuffer );
		m_DepthBuffer = 0;
	}

	if( m_Texture != GL_INVALID_TEXTURE_ID )
	{
		glDeleteTexture( m_Texture );
	}

	if( m_FrameBuffer != 0 )
	{
		glDeleteFramebuffers( 1, &m_FrameBuffer );
		m_FrameBuffer = 0;
	}
}

void GLRenderTarget::Setup( const GLsizei iWidth, const GLsizei iHeight, const bool bUseDepthBuffer )
{
	if( !Exists() || m_Texture == GL_INVALID_TEXTURE_ID )
		return;

	glBindTexture( GL_TEXTURE_2D, m_Texture );

	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, iWidth, iHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr );

	//Poor filtering, so it doesn't mess with the results.
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );

	//Unbind it so there can be no conflicts.
	glBindTexture( GL_TEXTURE_2D, GL_INVALID_TEXTURE_ID );

	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_Texture, 0 );

	const GLenum drawBuffer = GL_COLOR_ATTACHMENT0;

	glDrawBuffers( 1, &drawBuffer );

	//TODO: depth buffer doesn't work properly at this time.
	if( bUseDepthBuffer )
	{
		glBindRenderbuffer( GL_RENDERBUFFER, m_DepthBuffer );
		glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, iWidth, iHeight );

		glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_DepthBuffer );
		glBindRenderbuffer( GL_RENDERBUFFER, 0 );
	}
	else
	{
		glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0 );
	}
}

bool GLRenderTarget::Bind()
{
	if( m_FrameBuffer == 0 )
		return false;

	glBindFramebuffer( GL_FRAMEBUFFER, m_FrameBuffer );

	return true;
}

void GLRenderTarget::Unbind()
{
	if( m_FrameBuffer == 0 )
		return;

	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
}

GLenum GLRenderTarget::GetStatus() const
{
	return glCheckFramebufferStatus( GL_FRAMEBUFFER );
}

void GLRenderTarget::FinishDraw()
{
	glFlush();
	glFinish();
}

void GLRenderTarget::GetPixels( const GLsizei iWidth, const GLsizei iHeight, const GLenum format, const GLenum type, void* pPixels )
{
	glReadPixels( 0, 0, iWidth, iHeight, format, type, pPixels );
}