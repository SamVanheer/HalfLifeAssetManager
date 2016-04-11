#include "common/Logging.h"

#include "GraphicsHelpers.h"

namespace graphics
{
namespace helpers
{
/*
*	Sets up OpenGL for the specified render mode.
*	renderMode:		Render mode to set up. Must be valid.
*/
void SetupRenderMode( RenderMode renderMode )
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
			glEnable( GL_CULL_FACE );
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
			glEnable( GL_CULL_FACE );
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

/*
*	Draws a texture onto the screen. Optionally draws a UV map, either on a black background, or on top of the texture.
*	iWidth:				Width of the viewport
*	iHeight:			Height of the viewport
*	pStudioModel:		Model whose texture is being drawn
*	iTexture:			Index of the texture to draw
*	flTextureScale:		Zoom level
*	bShowUVMap:			If true, draws the UV map
*	bOverlayUVMap:		If true, and bShowUVMap is true, overlays the UV map on top of the texture
*	bAntiAliasLines:	If true, anti aliases UV map lines
*	pUVMesh:			If specified, is the mesh to use to draw the UV map. If null, all meshes that use the texture are drawn.
*/
void DrawTexture( const int iWidth, const int iHeight,
				  const StudioModel& studioModel,
				  const int iTexture, const float flTextureScale, const bool bShowUVMap, const bool bOverlayUVMap, const bool bAntiAliasLines, 
				  const mstudiomesh_t* const pUVMesh )
{
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();

	glOrtho( 0.0f, ( float ) iWidth, ( float ) iHeight, 0.0f, 1.0f, -1.0f );

	const studiohdr_t* const hdr = studioModel.getTextureHeader();
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
			glBindTexture( GL_TEXTURE_2D, studioModel.GetTextureId( iTexture ) );

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

			size_t uiCount;

			const mstudiomesh_t* const* ppMeshes;

			if( pUVMesh )
			{
				uiCount = 1;
				ppMeshes = &pUVMesh;
			}
			else
			{
				const StudioModel::MeshList_t* pList = studioModel.GetMeshListByTexture( iTexture );

				uiCount = pList->size();
				ppMeshes = pList->data();
			}

			SetupRenderMode( RenderMode::WIREFRAME );

			if( bAntiAliasLines )
			{
				glEnable( GL_BLEND );
				glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
				glEnable( GL_LINE_SMOOTH );
			}

			int i;

			for( size_t uiIndex = 0; uiIndex < uiCount; ++uiIndex, ++ppMeshes )
			{
				const short* ptricmds = ( short* ) ( ( byte* ) studioModel.getStudioHeader() + ( *ppMeshes )->triindex );

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

/*
*	Draws a background texture, fitted to the viewport.
*	backgroundTexture:	OpenGL texture id that represents the background texture
*/
void DrawBackground( GLuint backgroundTexture )
{
	if( backgroundTexture == GL_INVALID_TEXTURE_ID )
		return;

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();

	glOrtho( 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, -1.0f );

	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();
	glLoadIdentity();

	glDisable( GL_CULL_FACE );
	glEnable( GL_TEXTURE_2D );

	glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
	glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

	glBindTexture( GL_TEXTURE_2D, backgroundTexture );

	glBegin( GL_TRIANGLE_STRIP );

	glTexCoord2f( 0, 0 );
	glVertex2f( 0, 0 );

	glTexCoord2f( 1, 0 );
	glVertex2f( 1, 0 );

	glTexCoord2f( 0, 1 );
	glVertex2f( 0, 1 );

	glTexCoord2f( 1, 1 );
	glVertex2f( 1, 1 );

	glEnd();

	glPopMatrix();

	glClear( GL_DEPTH_BUFFER_BIT );
	glBindTexture( GL_TEXTURE_2D, 0 );
}

/*
*	Sets the projection matrix to the default perspective settings.
*	iWidth:		Width of the viewport
*	iHeight:	Height of the viewport
*/
void SetProjection( const int iWidth, const int iHeight )
{
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluPerspective( 65.0f, ( GLfloat ) iWidth / ( GLfloat ) iHeight, 1.0f, 4096.0f );
}

/*
*	Draws a floor quad.
*	flSideLength:	Length of one side of the floor
*/
void DrawFloorQuad( float flSideLength )
{
	flSideLength = abs( flSideLength );

	flSideLength /= 2;

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

/*
*	Draws a floor, optionally with a texture.
*	flSideLength:	Length of one side of the floor
*	groundTexture:	OpenGL texture id of the texture to draw as the floor, or GL_INVALID_TEXTURE_ID to draw a solid color instead
*	groundColor:	Color of the ground if no texture is specified
*	bMirror:		If true, draws a solid underside
*/
void DrawFloor( float flSideLength, GLuint groundTexture, const vec3_t groundColor, const bool bMirror )
{
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
		glColor4f( groundColor[ 0 ], groundColor[ 1 ], groundColor[ 2 ], 0.7f );
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

/*
*	Draws a wireframe overlay over a model
*	model:	Model to draw
*/
unsigned int DrawWireframeOverlay( StudioModel& model, const StudioModel::CRenderSettings& settings )
{
	SetupRenderMode( RenderMode::WIREFRAME );

	return model.DrawModel( settings, true );
}

/*
*	Draws a mirrored model.
*	model:				Model to draw
*	renderMode:			Render mode to use
*	bWireframeOverlay:	Whether to render a wireframe overlay on top of the model
*	flSideLength:		Length of one side of the floor
*/
unsigned int DrawMirroredModel( StudioModel& model, const RenderMode renderMode, const StudioModel::CRenderSettings& settings, const bool bWireframeOverlay, const float flSideLength )
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
	SetupRenderMode( renderMode );
	unsigned int uiDrawnPolys = model.DrawModel( settings );

	//Draw wireframe overlay
	//TODO: integrate this into DrawModel somehow.
	if( bWireframeOverlay )
	{
		uiDrawnPolys += DrawWireframeOverlay( model, settings );
	}

	glPopMatrix();

	glDisable( GL_STENCIL_TEST );

	return uiDrawnPolys;
}
}
}