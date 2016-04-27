#ifndef GRAPHICS_OPENGL_H
#define GRAPHICS_OPENGL_H

#include "shared/Platform.h"

//#include <gl/GL.h>
//#include <gl/GLU.h>

#include <GL/glew.h>

/**
*	Base class for OpenGL system. Handles GLEW initialization, and other operations that may depend on the UI being used.
*/
class CBaseOpenGL
{
public:
	/**
	*	@return Whether OpenGL has been post initialized or not.
	*/
	bool IsPostInitialized() const { return m_bPostInitialized; }

	/**
	*	Should be called after the context is created. If false, exit the program.
	*	@return true if Post initialization succeeds, false otherwise.
	*/
	bool PostInitialize();

	virtual GLuint glLoadImage( const char* const pszFilename ) = 0;

protected:
	CBaseOpenGL()
	{
	}

private:
	bool m_bPostInitialized = false;
	/**
	*	Only valid if m_bInitialized is true; contains the result of glewInit()
	*/
	GLenum m_GLEWResult;

private:
	CBaseOpenGL( const CBaseOpenGL& ) = delete;
	CBaseOpenGL& operator=( const CBaseOpenGL& ) = delete;
};

/**
*	As specified by the OpenGL standard, 0 is not a valid texture id.
*/
const GLuint GL_INVALID_TEXTURE_ID = 0;

void glDeleteTexture( GLuint& textureId );

const char* glFrameBufferStatusToString( const GLenum status );

#endif //GRAPHICS_OPENGL_H