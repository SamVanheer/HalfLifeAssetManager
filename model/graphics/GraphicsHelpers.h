#ifndef GRAPHICS_GRAPHICSHELPERS_H
#define GRAPHICS_GRAPHICSHELPERS_H

#include "model/studiomodel/StudioModel.h"

#include "model/graphics/Constants.h"

#include "model/utility/OpenGL.h"

namespace graphics
{
namespace helpers
{
void SetupRenderMode( RenderMode renderMode );

void DrawTexture( const int iWidth, const int iHeight,
				  const StudioModel& studioModel,
				  const int iTexture, const float flTextureScale, const bool bShowUVMap, const bool bOverlayUVMap, const bool bAntiAliasLines,
				  const mstudiomesh_t* const pUVMesh );

void DrawBackground( GLuint backgroundTexture );

void SetProjection( const int iWidth, const int iHeight );

void DrawFloorQuad( float flSideLength );

void DrawFloor( float flSideLength, GLuint groundTexture, const vec3_t groundColor, const bool bMirror );

unsigned int DrawWireframeOverlay( StudioModel& model );

unsigned int DrawMirroredModel( StudioModel& model, const RenderMode renderMode, const bool bWireframeOverlay, const float flSideLength );
}
}

#endif //GRAPHICS_GRAPHICSHELPERS_H