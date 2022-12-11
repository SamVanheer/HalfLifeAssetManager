#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <optional>
#include <string_view>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "graphics/GraphicsConstants.hpp"
#include "graphics/OpenGL.hpp"
#include "graphics/Palette.hpp"

class StudioModelEntity;

namespace studiomdl
{
class IStudioModelRenderer;
}

namespace graphics
{
/**
*	Converts an 8 bit image to a 24 bit RGB image.
*/
void Convert8to24Bit(const int iWidth, const int iHeight, const std::byte* const pData, const RGBPalette& palette, std::byte* const pOutData);

/**
*	Flips an image vertically. This allows conversion between OpenGL and image formats. The image is flipped in place.
*	@param iWidth Image width, in pixels.
*	@param iHeight Image height, in pixels.
*	@param pData Pixel data, in RGB 24 bit.
*/
void FlipImageVertically(const int iWidth, const int iHeight, std::byte* const pData);

/**
*	Draws a background texture, fitted to the viewport.
*	@param backgroundTexture OpenGL texture id that represents the background texture
*/
void DrawBackground(QOpenGLFunctions_1_1* openglFunctions, GLuint backgroundTexture);

inline std::array<glm::vec3, 8> CreateBoxFromBounds(const glm::vec3& min, const glm::vec3& max)
{
	return
	{
		{
			{min[0], max[1], min[2]},
			{min[0], min[1], min[2]},
			{max[0], max[1], min[2]},
			{max[0], min[1], min[2]},
			{max[0], max[1], max[2]},
			{max[0], min[1], max[2]},
			{min[0], max[1], max[2]},
			{min[0], min[1], max[2]}
		}
	};
}

/**
*	Draws a box using an array of 8 vectors as corner points.
*/
void DrawBox(QOpenGLFunctions_1_1* openglFunctions, const std::array<glm::vec3, 8>& points);

void DrawOutlinedBox(QOpenGLFunctions_1_1* openglFunctions,
	const std::array<glm::vec3, 8>& points, const glm::vec4& faceColor, const glm::vec4& borderColor);

/**
*	@brief Tests if the given filename is a remap name, and returns the remap ranges if so
*/
bool TryGetRemapColors(std::string_view fileName, int& low, int& mid, int& high);

void PaletteHueReplace(RGBPalette& palette, int newHue, int start, int end);

/*
*	Sets up OpenGL for the specified render mode.
*	@param renderMode Render mode to set up. Must be valid.
*	@param bBackfaceCulling Whether backface culling should be enabled or not.
*/
void SetupRenderMode(QOpenGLFunctions_1_1* openglFunctions, RenderMode renderMode, const bool bBackfaceCulling);

/**
*	Draws a floor quad.
*	@param origin Floor origin
*	@param floorLength Length of one side of the floor
*	@param textureRepeatLength Size of a texture repetition
*	@param textureOffset Offset in units to shift the texture
*/
void DrawFloorQuad(QOpenGLFunctions_1_1* openglFunctions, const glm::vec3& origin, float floorLength, float textureRepeatLength, glm::vec2 textureOffset);

/**
*	Draws a floor, optionally with a texture.
*	@param origin Floor origin
*	@param floorLength		Length of one side of the floor
*	@param textureRepeatLength Size of a texture repetition
*	@param textureOffset Offset in units to shift the texture
*	@param groundTexture	OpenGL texture id of the texture to draw as the floor, or GL_INVALID_TEXTURE_ID to draw a solid color instead
*	@param groundColor		Color of the ground if no texture is specified
*	@param bMirror			If true, draws a solid underside
*/
void DrawFloor(QOpenGLFunctions_1_1* openglFunctions, 
	const glm::vec3& origin, float floorLength, float textureRepeatLength, const glm::vec2& textureOffset, std::optional<GLuint> groundTexture,
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
unsigned int DrawMirroredModel(QOpenGLFunctions_1_1* openglFunctions, 
	studiomdl::IStudioModelRenderer& studioModelRenderer, StudioModelEntity* pEntity,
	const RenderMode renderMode, const bool bWireframeOverlay, const glm::vec3& origin, const float floorLength, const bool bBackfaceCulling);
}
