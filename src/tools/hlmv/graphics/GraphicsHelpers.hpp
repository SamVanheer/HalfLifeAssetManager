#pragma once

#include "shared/studiomodel/CStudioModel.hpp"
#include "game/entity/CStudioModelEntity.hpp"

#include "graphics/Constants.hpp"

#include "graphics/OpenGL.hpp"

class Color;

namespace studiomdl
{
class IStudioModelRenderer;
}

namespace graphics
{
namespace helpers
{
/*
*	Sets up OpenGL for the specified render mode.
*	@param renderMode Render mode to set up. Must be valid.
*	@param bBackfaceCulling Whether backface culling should be enabled or not.
*/
void SetupRenderMode( RenderMode renderMode, const bool bBackfaceCulling );

/**
*	Draws a floor quad.
*	@param flSideLength Length of one side of the floor
*/
void DrawFloorQuad( float flSideLength );

/**
*	Draws a floor, optionally with a texture.
*	@param flSideLength		Length of one side of the floor
*	@param groundTexture	OpenGL texture id of the texture to draw as the floor, or GL_INVALID_TEXTURE_ID to draw a solid color instead
*	@param groundColor		Color of the ground if no texture is specified
*	@param bMirror			If true, draws a solid underside
*/
void DrawFloor( float flSideLength, GLuint groundTexture, const Color& groundColor, const bool bMirror );

/**
*	Draws a mirrored model.
*	@param studioModelRenderer Renderer to use
*	@param pEntity				Entity to draw
*	@param renderMode			Render mode to use
*	@param bWireframeOverlay	Whether to render a wireframe overlay on top of the model
*	@param flSideLength			Length of one side of the floor
*	@param bBackfaceCulling		Whether to perform backface culling or not
*/
unsigned int DrawMirroredModel(studiomdl::IStudioModelRenderer& studioModelRenderer, CStudioModelEntity* pEntity,
	const RenderMode renderMode, const bool bWireframeOverlay, const float flSideLength, const bool bBackfaceCulling);
}
}
