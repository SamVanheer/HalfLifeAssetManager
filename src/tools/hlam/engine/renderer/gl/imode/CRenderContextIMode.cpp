#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "core/shared/Logging.hpp"

#include "graphics/OpenGL.hpp"

#include "CRenderContextIMode.hpp"

namespace renderer
{
void CRenderContextIMode::MatrixMode( const MatrixMode::MatrixMode mode )
{
	BaseClass::MatrixMode( mode );

	GLenum glMode;

	switch( mode )
	{
	case MatrixMode::MODEL:
		{
			glMode = GL_MODELVIEW;
			break;
		}

	case MatrixMode::VIEW:
		{
			glMode = GL_MODELVIEW;
			break;
		}

	case MatrixMode::PROJECTION:
		{
			glMode = GL_PROJECTION;
			break;
		}

	default:
		{
			//This should never happen, unless a new mode was added.
			Error( "CRenderContextGL1_5::MatrixMode: Invalid matrix mode \"%d\" specified!\n", mode );
			return;
		}
	}

	glMatrixMode( glMode );
}

void CRenderContextIMode::PushMatrix()
{
	BaseClass::PushMatrix();

	glPushMatrix();
}

void CRenderContextIMode::PopMatrix()
{
	BaseClass::PopMatrix();

	glPopMatrix();
}

void CRenderContextIMode::LoadIdentity()
{
	BaseClass::LoadIdentity();

	glLoadIdentity();
}

void CRenderContextIMode::LoadMatrix( const glm::mat4x4& mat )
{
	BaseClass::LoadMatrix( mat );

	const auto mode = GetMatrixMode();

	//Model and View need to be combined.
	if( mode == MatrixMode::MODEL )
	{
		glm::mat4x4 newMat = mat * GetMatrixStack().GetMatrix( MatrixMode::VIEW );

		glLoadMatrixf( glm::value_ptr( newMat ) );
	}
	else if( mode == MatrixMode::VIEW )
	{
		glm::mat4x4 newMat = GetMatrixStack().GetMatrix( MatrixMode::MODEL ) * mat;

		glLoadMatrixf( glm::value_ptr( newMat ) );
	}
	else
	{
		//Just a normal matrix.
		glLoadMatrixf( glm::value_ptr( mat ) );
	}
}

void CRenderContextIMode::LoadTransposeMatrix( const glm::mat4x4& mat )
{
	BaseClass::LoadTransposeMatrix( mat );

	const auto mode = GetMatrixMode();

	//Transposing here and handling it like a regular matrix will have the same effect.
	glm::mat4x4 transMat = glm::transpose( mat );

	//Model and View need to be combined.
	if( mode == MatrixMode::MODEL )
	{
		glm::mat4x4 newMat = transMat * GetMatrixStack().GetMatrix( MatrixMode::VIEW );

		glLoadMatrixf( glm::value_ptr( newMat ) );
	}
	else if( mode == MatrixMode::VIEW )
	{
		glm::mat4x4 newMat = GetMatrixStack().GetMatrix( MatrixMode::MODEL ) * transMat;

		glLoadMatrixf( glm::value_ptr( newMat ) );
	}
	else
	{
		//Just a normal matrix.
		glLoadMatrixf( glm::value_ptr( transMat ) );
	}
}

void CRenderContextIMode::MultMatrix( const glm::mat4x4& mat )
{
	BaseClass::MultMatrix( mat );

	const auto mode = GetMatrixMode();

	//Model and View need to be combined.
	if( mode == MatrixMode::MODEL )
	{
		glm::mat4x4 newMat = GetMatrixStack().GetMatrix( MatrixMode::MODEL ) * mat * GetMatrixStack().GetMatrix( MatrixMode::VIEW );

		glMultMatrixf( glm::value_ptr( newMat ) );
	}
	else if( mode == MatrixMode::VIEW )
	{
		glm::mat4x4 newMat = GetMatrixStack().GetMatrix( MatrixMode::MODEL ) * GetMatrixStack().GetMatrix( MatrixMode::VIEW ) * mat;

		glMultMatrixf( glm::value_ptr( newMat ) );
	}
	else
	{
		//Just a normal matrix.
		glMultMatrixf( glm::value_ptr( mat ) );
	}
}

void CRenderContextIMode::MultTransposeMatrix( const glm::mat4x4& mat )
{
	BaseClass::MultTransposeMatrix( mat );

	const auto mode = GetMatrixMode();

	//Transposing here and handling it like a regular matrix will have the same effect.
	glm::mat4x4 transMat = glm::transpose( mat );

	//Model and View need to be combined.
	if( mode == MatrixMode::MODEL )
	{
		glm::mat4x4 newMat = GetMatrixStack().GetMatrix( MatrixMode::MODEL ) * transMat * GetMatrixStack().GetMatrix( MatrixMode::VIEW );

		glMultMatrixf( glm::value_ptr( newMat ) );
	}
	else if( mode == MatrixMode::VIEW )
	{
		glm::mat4x4 newMat = GetMatrixStack().GetMatrix( MatrixMode::MODEL ) * GetMatrixStack().GetMatrix( MatrixMode::VIEW ) * transMat;

		glMultMatrixf( glm::value_ptr( newMat ) );
	}
	else
	{
		//Just a normal matrix.
		glMultMatrixf( glm::value_ptr( transMat ) );
	}
}
}