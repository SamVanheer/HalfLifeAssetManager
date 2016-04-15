#ifndef GRAPHICS_GRAPHICSHELPERS_H
#define GRAPHICS_GRAPHICSHELPERS_H

#include "studiomodel/StudioModel.h"

#include "graphics/Constants.h"

#include "graphics/OpenGL.h"

class Color;

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

void DrawFloor( float flSideLength, GLuint groundTexture, const Color& groundColor, const bool bMirror );

unsigned int DrawWireframeOverlay( StudioModel& model, const StudioModel::CRenderSettings& settings );

unsigned int DrawMirroredModel( StudioModel& model, const RenderMode renderMode, const StudioModel::CRenderSettings& settings, const bool bWireframeOverlay, const float flSideLength );
}
}

#endif //GRAPHICS_GRAPHICSHELPERS_H