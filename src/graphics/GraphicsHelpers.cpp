#include <cassert>

#include <glm/gtc/type_ptr.hpp>

#include "shared/Logging.h"

#include "utility/Color.h"

#include "shared/studiomodel/IStudioModelRenderer.h"

#include "GraphicsHelpers.h"

//TODO: remove
extern studiomdl::IStudioModelRenderer* g_pStudioMdlRenderer;

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

void DrawTexture( const int iWidth, const int iHeight,
				  CStudioModelEntity* pEntity,
				  const int iTexture, const float flTextureScale, const bool bShowUVMap, const bool bOverlayUVMap, const bool bAntiAliasLines, 
				  const mstudiomesh_t* const pUVMesh )
{
	assert( pEntity );

	auto pModel = pEntity->GetModel();

	assert( pModel );

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();

	glOrtho( 0.0f, ( float ) iWidth, ( float ) iHeight, 0.0f, 1.0f, -1.0f );

	const studiohdr_t* const hdr = pModel->GetTextureHeader();

	if( hdr )
	{
		mstudiotexture_t *ptextures = ( mstudiotexture_t * ) ( ( byte * ) hdr + hdr->textureindex );

		const mstudiotexture_t& texture = ptextures[ iTexture ];

		float w = ( float ) texture.width * flTextureScale;
		float h = ( float ) texture.height * flTextureScale;

		glMatrixMode( GL_MODELVIEW );
		glPushMatrix();
		glLoadIdentity();

		glDisable( GL_CULL_FACE );
		glDisable( GL_BLEND );

		if( texture.flags & STUDIO_NF_MASKED )
		{
			glEnable( GL_ALPHA_TEST );
			glAlphaFunc( GL_GREATER, 0.0f );
		}

		glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
		float x = ( ( float ) iWidth - w ) / 2;
		float y = ( ( float ) iHeight - h ) / 2;

		glDisable( GL_DEPTH_TEST );

		if( bShowUVMap && !bOverlayUVMap )
		{
			glColor4f( 0.0f, 0.0f, 0.0f, 1.0f );
			glDisable( GL_TEXTURE_2D );
			glRectf( x, y, x + w, y + h );
		}

		if( !bShowUVMap || bOverlayUVMap )
		{
			glEnable( GL_TEXTURE_2D );
			glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
			glBindTexture( GL_TEXTURE_2D, pModel->GetTextureId( iTexture ) );

			glBegin( GL_TRIANGLE_STRIP );

			glTexCoord2f( 0, 0 );
			glVertex2f( x, y );

			glTexCoord2f( 1, 0 );
			glVertex2f( x + w, y );

			glTexCoord2f( 0, 1 );
			glVertex2f( x, y + h );

			glTexCoord2f( 1, 1 );
			glVertex2f( x + w, y + h );

			glEnd();

			glBindTexture( GL_TEXTURE_2D, 0 );
		}

		if( bShowUVMap )
		{
			glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );

			CStudioModelEntity::MeshList_t meshes;

			if( pUVMesh )
			{
				meshes.push_back( pUVMesh );
			}
			else
			{
				meshes = pEntity->ComputeMeshList( iTexture );
			}

			SetupRenderMode( RenderMode::WIREFRAME, true );

			if( bAntiAliasLines )
			{
				glEnable( GL_BLEND );
				glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
				glEnable( GL_LINE_SMOOTH );
			}

			int i;

			const mstudiomesh_t* const* ppMeshes = meshes.data();

			for( size_t uiIndex = 0; uiIndex < meshes.size(); ++uiIndex, ++ppMeshes )
			{
				const short* ptricmds = ( short* ) ( ( byte* ) pModel->GetStudioHeader() + ( *ppMeshes )->triindex );

				while( i = *( ptricmds++ ) )
				{
					if( i < 0 )
					{
						glBegin( GL_TRIANGLE_FAN );
						i = -i;
					}
					else
					{
						glBegin( GL_TRIANGLE_STRIP );
					}

					for( ; i > 0; i--, ptricmds += 4 )
					{
						// FIX: put these in as integer coords, not floats
						glVertex2f( x + ptricmds[ 2 ] * flTextureScale, y + ptricmds[ 3 ] * flTextureScale );
					}
					glEnd();
				}
			}

			if( bAntiAliasLines )
			{
				glDisable( GL_LINE_SMOOTH );
			}
		}

		glPopMatrix();

		glClear( GL_DEPTH_BUFFER_BIT );

		if( texture.flags & STUDIO_NF_MASKED )
			glDisable( GL_ALPHA_TEST );
	}
}

void DrawFloorQuad( float flSideLength )
{
	flSideLength = abs( flSideLength );

	glBegin( GL_TRIANGLE_STRIP );
	glTexCoord2f( 0.0f, 0.0f );
	glVertex3f( -flSideLength, flSideLength, 0.0f );

	glTexCoord2f( 0.0f, 1.0f );
	glVertex3f( -flSideLength, -flSideLength, 0.0f );

	glTexCoord2f( 1.0f, 0.0f );
	glVertex3f( flSideLength, flSideLength, 0.0f );

	glTexCoord2f( 1.0f, 1.0f );
	glVertex3f( flSideLength, -flSideLength, 0.0f );

	glEnd();
}

void DrawFloor( float flSideLength, GLuint groundTexture, const Color& groundColor, const bool bMirror )
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
		glColor4f( groundColor[ 0 ] / 255.0f, groundColor[ 1 ] / 255.0f, groundColor[ 2 ] / 255.0f, 0.7f );
		glBindTexture( GL_TEXTURE_2D, 0 );
	}
	else
	{
		glEnable( GL_TEXTURE_2D );
		glColor4f( 1.0f, 1.0f, 1.0f, 0.6f );
		glBindTexture( GL_TEXTURE_2D, groundTexture );
	}

	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	graphics::helpers::DrawFloorQuad( flSideLength );

	glDisable( GL_BLEND );

	if( bMirror )
	{
		glCullFace( GL_BACK );
		glColor4f( 0.1f, 0.1f, 0.1f, 1.0f );
		glBindTexture( GL_TEXTURE_2D, 0 );
		graphics::helpers::DrawFloorQuad( flSideLength );

		glFrontFace( GL_CCW );
	}
	else
		glEnable( GL_CULL_FACE );
}

unsigned int DrawWireframeOverlay( CStudioModelEntity* pEntity )
{
	SetupRenderMode( RenderMode::WIREFRAME, true );

	const unsigned int uiOldPolys = g_pStudioMdlRenderer->GetDrawnPolygonsCount();

	pEntity->Draw( entity::DRAWF_WIREFRAME_ONLY );

	return g_pStudioMdlRenderer->GetDrawnPolygonsCount() - uiOldPolys;
}

unsigned int DrawMirroredModel( CStudioModelEntity* pEntity, const RenderMode renderMode, const bool bWireframeOverlay, const float flSideLength, const bool bBackfaceCulling )
{
	/* Don't update color or depth. */
	glDisable( GL_DEPTH_TEST );
	glColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE );

	/* Draw 1 into the stencil buffer. */
	glEnable( GL_STENCIL_TEST );
	glStencilOp( GL_KEEP, GL_KEEP, GL_REPLACE );
	glStencilFunc( GL_ALWAYS, 1, 0xffffffff );

	/* Now render floor; floor pixels just get their stencil set to 1. */
	graphics::helpers::DrawFloorQuad( flSideLength );

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

	const unsigned int uiOldPolys = g_pStudioMdlRenderer->GetDrawnPolygonsCount();

	pEntity->Draw( entity::DRAWF_NONE );

	//Draw wireframe overlay
	if( bWireframeOverlay )
	{
		DrawWireframeOverlay( pEntity );
	}

	glDisable( GL_CLIP_PLANE0 );

	glPopMatrix();

	glDisable( GL_STENCIL_TEST );

	return g_pStudioMdlRenderer->GetDrawnPolygonsCount() - uiOldPolys;
}
}
}