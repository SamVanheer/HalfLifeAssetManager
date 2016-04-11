#include "common/Logging.h"

#include "OpenGL.h"

bool CBaseOpenGL::PostInitialize()
{
	if( IsPostInitialized() )
	{
		//Should only get here if it was already initalized.
		return GLEW_OK == m_GLEWResult;
	}

	m_bPostInitialized = true;

	m_GLEWResult = glewInit();

	if( m_GLEWResult != GLEW_OK )
	{
		Error( "Error initializing GLEW:\n%s", reinterpret_cast<const char*>( glewGetErrorString( m_GLEWResult ) ) );
	}

	return GLEW_OK == m_GLEWResult;
}

void glDeleteTexture( GLuint& textureId )
{
	if( textureId != GL_INVALID_TEXTURE_ID )
	{
		glDeleteTextures( 1, &textureId );
		textureId = GL_INVALID_TEXTURE_ID;
	}
}