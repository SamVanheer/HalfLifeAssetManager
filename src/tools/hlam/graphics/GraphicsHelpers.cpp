#include <cassert>
#include <cmath>

#include <glm/vec4.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shared/Logging.hpp"

#include "shared/renderer/studiomodel/IStudioModelRenderer.hpp"

#include "GraphicsHelpers.hpp"

namespace graphics
{
namespace helpers
{
void SetupRenderMode( RenderMode renderMode, const bool bBackfaceCulling )
{
	if( renderMode == RenderMode::INVALID )
		return;

	switch( renderMode )
	{
	case RenderMode::WIREFRAME:
		{
			glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
			glDisable( GL_TEXTURE_2D );
			glDisable( GL_CULL_FACE );
			glEnable( GL_DEPTH_TEST );

			break;
		}

	case RenderMode::FLAT_SHADED:
	case RenderMode::SMOOTH_SHADED:
		{
			glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
			glDisable( GL_TEXTURE_2D );

			if( bBackfaceCulling )
			{
				glEnable( GL_CULL_FACE );
			}
			else
			{
				glDisable( GL_CULL_FACE );
			}

			glEnable( GL_DEPTH_TEST );

			if( renderMode == RenderMode::FLAT_SHADED )
				glShadeModel( GL_FLAT );
			else
				glShadeModel( GL_SMOOTH );

			break;
		}

	case RenderMode::TEXTURE_SHADED:
		{
			glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
			glEnable( GL_TEXTURE_2D );

			if( bBackfaceCulling )
			{
				glEnable( GL_CULL_FACE );
			}
			else
			{
				glDisable( GL_CULL_FACE );
			}

			glEnable( GL_DEPTH_TEST );
			glShadeModel( GL_SMOOTH );

			break;
		}

	default:
		{
			Warning( "graphics::helpers::SetupRenderMode: Invalid render mode %d\n", static_cast<int>( renderMode ) );
			break;
		}
	}
}

void DrawFloorQuad(float floorLength, float textureRepeatLength, glm::vec2 textureOffset)
{
	const float vertexCoord{floorLength / 2};

	//The texture should repeat so that a textureRepeatLength sized quad contains one repetition of the texture
	//It should also, when movement distance is 0, repeat at 0, 0, 0
	const float repetition = vertexCoord / textureRepeatLength;

	const float textureMax = repetition;
	const float textureMin = -repetition;

	//Rescale offset to match the texture size
	textureOffset /= textureRepeatLength;

	glBegin(GL_TRIANGLE_STRIP);
	glTexCoord2f(textureMin + textureOffset.x, textureMin + textureOffset.y);
	glVertex3f(-vertexCoord, vertexCoord, 0.0f);

	glTexCoord2f(textureMin + textureOffset.x, textureMax + textureOffset.y);
	glVertex3f(-vertexCoord, -vertexCoord, 0.0f);

	glTexCoord2f(textureMax + textureOffset.x, textureMin + textureOffset.y);
	glVertex3f(vertexCoord, vertexCoord, 0.0f);

	glTexCoord2f(textureMax + textureOffset.x, textureMax + textureOffset.y);
	glVertex3f(vertexCoord, -vertexCoord, 0.0f);

	glEnd();
}

void DrawFloor(float floorLength, float textureRepeatLength, const glm::vec2& textureOffset, GLuint groundTexture, const glm::vec3& groundColor, const bool bMirror)
{
	glCullFace( GL_FRONT );

	glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
	glEnable( GL_DEPTH_TEST );
	glEnable( GL_CULL_FACE );

	if( bMirror )
		glFrontFace( GL_CW );
	else
		glDisable( GL_CULL_FACE );

	glEnable( GL_BLEND );
	if( groundTexture == GL_INVALID_TEXTURE_ID )
	{
		glDisable( GL_TEXTURE_2D );
		glColor4fv(glm::value_ptr(glm::vec4{groundColor, 0.7}));
		glBindTexture( GL_TEXTURE_2D, 0 );
	}
	else
	{
		glEnable( GL_TEXTURE_2D );
		glColor4f( 1.0f, 1.0f, 1.0f, 0.6f );
		glBindTexture( GL_TEXTURE_2D, groundTexture );
	}

	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	graphics::helpers::DrawFloorQuad(floorLength, textureRepeatLength, textureOffset);

	glDisable( GL_BLEND );

	if( bMirror )
	{
		glCullFace( GL_BACK );
		glColor4f( 0.1f, 0.1f, 0.1f, 1.0f );
		glBindTexture( GL_TEXTURE_2D, 0 );
		graphics::helpers::DrawFloorQuad(floorLength, textureRepeatLength, textureOffset);

		glFrontFace( GL_CCW );
	}
	else
		glEnable( GL_CULL_FACE );
}

unsigned int DrawMirroredModel(studiomdl::IStudioModelRenderer& studioModelRenderer, StudioModelEntity* pEntity,
	const RenderMode renderMode, const bool bWireframeOverlay, const float floorLength, const bool bBackfaceCulling)
{
	/* Don't update color or depth. */
	glDisable( GL_DEPTH_TEST );
	glColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE );

	/* Draw 1 into the stencil buffer. */
	glEnable( GL_STENCIL_TEST );
	glStencilOp( GL_KEEP, GL_KEEP, GL_REPLACE );
	glStencilFunc( GL_ALWAYS, 1, 0xffffffff );

	/* Now render floor; floor pixels just get their stencil set to 1. */
	//Texture length is irrelevant here
	graphics::helpers::DrawFloorQuad(floorLength, 1, glm::vec2{0});

	/* Re-enable update of color and depth. */
	glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
	glEnable( GL_DEPTH_TEST );

	/* Now, only render where stencil is set to 1. */
	glStencilFunc( GL_EQUAL, 1, 0xffffffff );  /* draw if ==1 */
	glStencilOp( GL_KEEP, GL_KEEP, GL_KEEP );

	//Note: setting the stencil mask to 0 here would essentially disable the stencil buffer.

	glPushMatrix();
	glScalef( 1, 1, -1 );
	glCullFace( GL_BACK );
	SetupRenderMode( renderMode, bBackfaceCulling );

	glEnable( GL_CLIP_PLANE0 );

	/*
	*	This defines a clipping plane that covers the ground. Any mirrored polygons will not be drawn above the ground.
	*/
	const GLdouble flClipPlane[] = 
	{
		0.0,	//X
		0.0,	//Y
		1.0,	//Z
		0.0		//Offset in direction. In our case, this would move the plane up or down.
	};

	glClipPlane( GL_CLIP_PLANE0, flClipPlane );

	const glm::vec3& vecScale = pEntity->GetScale();

	//Determine if an odd number of scale values are negative. The cull face has to be changed if so.
	const float flScale = vecScale.x * vecScale.y * vecScale.z;

	glCullFace( flScale > 0 ? GL_BACK : GL_FRONT );

	const unsigned int uiOldPolys = studioModelRenderer.GetDrawnPolygonsCount();

	renderer::DrawFlags_t flags = renderer::DrawFlag::NONE;

	//Draw wireframe overlay
	if( bWireframeOverlay )
	{
		flags |= renderer::DrawFlag::WIREFRAME_OVERLAY;
	}

	pEntity->Draw( flags );

	glDisable( GL_CLIP_PLANE0 );

	glPopMatrix();

	glDisable( GL_STENCIL_TEST );

	return studioModelRenderer.GetDrawnPolygonsCount() - uiOldPolys;
}
}
}
