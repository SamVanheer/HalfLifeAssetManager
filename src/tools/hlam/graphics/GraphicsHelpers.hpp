#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "shared/studiomodel/CStudioModel.hpp"
#include "game/entity/StudioModelEntity.hpp"

#include "graphics/Constants.hpp"

#include "graphics/OpenGL.hpp"

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
*	@param floorLength Length of one side of the floor
*	@param textureRepeatLength Size of a texture repetition
*	@param textureOffset Offset in units to shift the texture
*/
void DrawFloorQuad(float floorLength, float textureRepeatLength, glm::vec2 textureOffset);

/**
*	Draws a floor, optionally with a texture.
*	@param floorLength		Length of one side of the floor
*	@param textureRepeatLength Size of a texture repetition
*	@param textureOffset Offset in units to shift the texture
*	@param groundTexture	OpenGL texture id of the texture to draw as the floor, or GL_INVALID_TEXTURE_ID to draw a solid color instead
*	@param groundColor		Color of the ground if no texture is specified
*	@param bMirror			If true, draws a solid underside
*/
void DrawFloor(float floorLength, float textureRepeatLength, const glm::vec2& textureOffset, GLuint groundTexture,
	const glm::vec3& groundColor, const bool bMirror);

/**
*	Draws a mirrored model.
*	@param studioModelRenderer Renderer to use
*	@param pEntity				Entity to draw
*	@param renderMode			Render mode to use
*	@param bWireframeOverlay	Whether to render a wireframe overlay on top of the model
*	@param floorLength			Length of one side of the floor
*	@param bBackfaceCulling		Whether to perform backface culling or not
*/
unsigned int DrawMirroredModel(studiomdl::IStudioModelRenderer& studioModelRenderer, StudioModelEntity* pEntity,
	const RenderMode renderMode, const bool bWireframeOverlay, const float floorLength, const bool bBackfaceCulling);
}
}
