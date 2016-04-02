#include <ui/wxInclude.h>

#include <memory>

#include <wx/image.h>

#include "OpenGL.h"

GLuint glLoadImage( const char* const pszFilename )
{
	if( !pszFilename || !( *pszFilename ) )
		return GL_INVALID_TEXTURE_ID;

	if( !wxFileExists( pszFilename ) )
	{
		wxMessageBox( wxString::Format( "File \"%s\" does not exist\n", pszFilename ) );
		return GL_INVALID_TEXTURE_ID;
	}

	wxImage image( pszFilename );

	if( !image.IsOk() )
	{
		wxMessageBox( wxString::Format( "An error occurred while loading \"%s\"\n", pszFilename ) );
		return GL_INVALID_TEXTURE_ID;
	}

	const unsigned char* const pData = image.GetData();
	const unsigned char* const pAlpha = image.GetAlpha();

	const int BPP = image.HasAlpha() ? 4 : 3;

	std::unique_ptr<GLubyte[]> pImageData( new GLubyte[ image.GetWidth() * image.GetHeight() * BPP ] );

	const int iHeight = image.GetHeight() - 1;

	for( int y = 0; y < image.GetHeight(); ++y )
	{
		for( int x = 0; x < image.GetWidth(); ++x )
		{
			for( int i = 0; i < 3; ++i )
			{
				pImageData[ ( x + y * image.GetWidth() ) * BPP + i ] = pData[ ( x + ( y ) * image.GetWidth() ) * 3 + i ];
			}

			if( image.HasAlpha() )
			{
				pImageData[ ( x + y * image.GetWidth() ) * BPP + 3 ] = pAlpha[ x + ( y ) * image.GetWidth() ];
			}
		}
	}

	GLuint textureId;

	glGenTextures( 1, &textureId );

	glBindTexture( GL_TEXTURE_2D, textureId );
	glTexImage2D( GL_TEXTURE_2D, 0, 3, image.GetWidth(), image.GetHeight(), 0, image.HasAlpha() ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, pImageData.get() );
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

	return textureId;
}

void glDeleteTexture( GLuint& textureId )
{
	if( textureId != GL_INVALID_TEXTURE_ID )
	{
		glDeleteTextures( 1, &textureId );
		textureId = GL_INVALID_TEXTURE_ID;
	}
}