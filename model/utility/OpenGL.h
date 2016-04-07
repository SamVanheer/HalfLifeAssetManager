#ifndef OPENGL_H
#define OPENGL_H

#include "Platform.h"

//#include <gl/GL.h>
//#include <gl/GLU.h>

#include <GL/glew.h>

//Should be called after a context is created. If false, exit the program.
bool PostInitializeOpenGL();

/*
*	As specified by the OpenGL standard, 0 is not a valid texture id.
*/
const GLuint GL_INVALID_TEXTURE_ID = 0;

GLuint glLoadImage( const char* const pszFilename );

void glDeleteTexture( GLuint& textureId );

#endif //OPENGL_H