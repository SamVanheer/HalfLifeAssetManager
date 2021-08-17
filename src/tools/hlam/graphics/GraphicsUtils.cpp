#include <algorithm>
#include <cassert>
#include <charconv>
#include <cmath>
#include <cstring>

#include <glm/vec4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "core/shared/Const.hpp"
#include "core/shared/Platform.hpp"

#include "engine/shared/renderer/studiomodel/IStudioModelRenderer.hpp"

#include "game/entity/StudioModelEntity.hpp"

#include "graphics/GraphicsUtils.hpp"
#include "graphics/Palette.hpp"

namespace graphics
{
void Convert8to24Bit( const int iWidth, const int iHeight, const byte* const pData, const RGBPalette& palette, byte* const pOutData )
{
	assert( pData );
	assert( pOutData );

	byte* pOut = pOutData;

	for( int y = 0; y < iHeight; ++y )
	{
		for( int x = 0; x < iWidth; ++x, pOut += 3 )
		{
			pOut[0] = palette[pData[x + y * iWidth]].R;
			pOut[1] = palette[pData[x + y * iWidth]].G;
			pOut[2] = palette[pData[x + y * iWidth]].B;
		}
	}
}

void FlipImageVertically( const int iWidth, const int iHeight, byte* const pData )
{
	assert( iWidth > 0 );
	assert( iHeight > 0 );
	assert( pData );

	const int iHalfHeight = iHeight / 2;

	for( int y = iHalfHeight; y < iHeight; ++y )
	{
		for( int x = 0; x < iWidth; ++x )
		{
			for( int i = 0; i < 3; ++i )
			{
				std::swap( pData[ ( x + y * iWidth ) * 3 + i ], pData[ ( x + ( iHeight - y - 1 ) * iWidth ) * 3 + i ] );
			}
		}
	}
}

void DrawBackground( GLuint backgroundTexture )
{
	if( backgroundTexture == GL_INVALID_TEXTURE_ID )
		return;

	glDisable(GL_BLEND);

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

void DrawBox(const std::array<glm::vec3, 8>& points)
{
	glBegin(GL_QUAD_STRIP);
	for (int i = 0; i < 10; ++i)
	{
		glVertex3fv(glm::value_ptr(points[i & 7]));
	}
	glEnd();

	glBegin(GL_QUAD_STRIP);
	glVertex3fv(glm::value_ptr(points[6]));
	glVertex3fv(glm::value_ptr(points[0]));
	glVertex3fv(glm::value_ptr(points[4]));
	glVertex3fv(glm::value_ptr(points[2]));
	glEnd();

	glBegin(GL_QUAD_STRIP);
	glVertex3fv(glm::value_ptr(points[1]));
	glVertex3fv(glm::value_ptr(points[7]));
	glVertex3fv(glm::value_ptr(points[3]));
	glVertex3fv(glm::value_ptr(points[5]));
	glEnd();
}

void DrawOutlinedBox(const std::array<glm::vec3, 8>& points, const glm::vec4& faceColor, const glm::vec4& borderColor)
{
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//Cull interior faces to avoid overlap
	glEnable(GL_CULL_FACE);

	//Disable depth mask so lines can draw over the box itself
	GLint depthMaskWasEnabled = GL_FALSE;

	glGetIntegerv(GL_DEPTH_WRITEMASK, &depthMaskWasEnabled);

	glDepthMask(GL_FALSE);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glColor4fv(glm::value_ptr(faceColor));

	DrawBox(points);

	glDepthMask(GL_TRUE);

	//Draw edges
	glDisable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glColor4fv(glm::value_ptr(borderColor));

	DrawBox(points);

	glDepthMask(depthMaskWasEnabled ? GL_TRUE : GL_FALSE);
}

const std::string_view DmBaseName{"DM_Base.bmp"};
const std::string_view RemapName{"Remap"};

const std::size_t SimpleRemapLength = 18;
const std::size_t FullRemapLength = 22;

const std::size_t LowOffset = 7;
const std::size_t MidOffset = 11;
const std::size_t HighOffset = 15;
const std::size_t ValueLength = 3;

bool TryGetRemapColors(std::string_view fileName, int& low, int& mid, int& high)
{
	if (fileName.length() == DmBaseName.length() &&
		!strncasecmp(fileName.data(), DmBaseName.data(), DmBaseName.length()))
	{
		low = 160;
		mid = 191;
		high = 223;

		return true;
	}
	else if ((fileName.length() == SimpleRemapLength || fileName.length() == FullRemapLength) &&
		!strncasecmp(fileName.data(), RemapName.data(), RemapName.length()))
	{
		//from_chars does not set the out value unless parsing succeeds, unlike atoi which the engine uses
		low = mid = high = 0;

		if (fileName.length() == SimpleRemapLength)
		{
			const auto index = fileName[RemapName.length()];

			if (index != 'c' && index != 'C')
			{
				return false;
			}
		}
		else
		{
			std::from_chars(fileName.data() + HighOffset, fileName.data() + HighOffset + ValueLength, high);
		}

		std::from_chars(fileName.data() + LowOffset, fileName.data() + LowOffset + ValueLength, low);
		std::from_chars(fileName.data() + MidOffset, fileName.data() + MidOffset + ValueLength, mid);

		//Clamp to valid range
		low = std::clamp(low, 0, 255);
		mid = std::clamp(mid, 0, 255);
		high = std::clamp(high, 0, 255);

		return true;
	}

	return false;
}

void PaletteHueReplace(RGBPalette& palette, int newHue, int start, int end)
{
	const auto hue = (float) (newHue * (360.0 / 255));

	for (int i = start; i <= end; ++i)
	{
		float r = palette[i].R;
		float g = palette[i].G;
		float b = palette[i].B;

		const auto maxcol = std::max({r, g, b}) / 255.0f;
		auto mincol = std::min({r, g, b}) / 255.0f;

		const auto val = maxcol;
		const auto sat = (maxcol - mincol) / maxcol;

		mincol = val * (1.0f - sat);

		if (hue <= 120)
		{
			b = mincol;
			if (hue < 60)
			{
				r = val;
				g = mincol + hue * (val - mincol) / (120 - hue);
			}
			else
			{
				g = val;
				r = mincol + (120 - hue) * (val - mincol) / hue;
			}
		}
		else if (hue <= 240)
		{
			r = mincol;
			if (hue < 180)
			{
				g = val;
				b = mincol + (hue - 120) * (val - mincol) / (240 - hue);
			}
			else
			{
				b = val;
				g = mincol + (240 - hue) * (val - mincol) / (hue - 120);
			}
		}
		else
		{
			g = mincol;
			if (hue < 300)
			{
				b = val;
				r = mincol + (hue - 240) * (val - mincol) / (360 - hue);
			}
			else
			{
				r = val;
				b = mincol + (360 - hue) * (val - mincol) / (hue - 240);
			}
		}

		palette[i].R = (byte) (r * 255);
		palette[i].G = (byte) (g * 255);
		palette[i].B = (byte) (b * 255);
	}
}

void SetupRenderMode(RenderMode renderMode, const bool bBackfaceCulling)
{
	if (renderMode == RenderMode::INVALID)
		return;

	switch (renderMode)
	{
	case RenderMode::WIREFRAME:
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);

		break;
	}

	case RenderMode::FLAT_SHADED:
	case RenderMode::SMOOTH_SHADED:
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glDisable(GL_TEXTURE_2D);

		if (bBackfaceCulling)
		{
			glEnable(GL_CULL_FACE);
		}
		else
		{
			glDisable(GL_CULL_FACE);
		}

		glEnable(GL_DEPTH_TEST);

		if (renderMode == RenderMode::FLAT_SHADED)
			glShadeModel(GL_FLAT);
		else
			glShadeModel(GL_SMOOTH);

		break;
	}

	case RenderMode::TEXTURE_SHADED:
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glEnable(GL_TEXTURE_2D);

		if (bBackfaceCulling)
		{
			glEnable(GL_CULL_FACE);
		}
		else
		{
			glDisable(GL_CULL_FACE);
		}

		glEnable(GL_DEPTH_TEST);
		glShadeModel(GL_SMOOTH);

		break;
	}
	}
}

void DrawFloorQuad(const glm::vec3& origin, float floorLength, float textureRepeatLength, glm::vec2 textureOffset)
{
	const float vertexCoord{floorLength / 2};

	//The texture should repeat so that a textureRepeatLength sized quad contains one repetition of the texture
	//It should also, when movement distance is 0, repeat at 0, 0, 0
	const float repetition = vertexCoord / textureRepeatLength;

	const float textureMax = repetition + 0.5f;
	const float textureMin = -repetition + 0.5f;

	//Rescale offset to match the texture size
	textureOffset /= textureRepeatLength;

	glBegin(GL_TRIANGLE_STRIP);
	glTexCoord2f(textureMin + textureOffset.x, textureMin + textureOffset.y);
	glVertex3f(origin.x - vertexCoord, origin.y + vertexCoord, origin.z);

	glTexCoord2f(textureMin + textureOffset.x, textureMax + textureOffset.y);
	glVertex3f(origin.x - vertexCoord, origin.y - vertexCoord, origin.z);

	glTexCoord2f(textureMax + textureOffset.x, textureMin + textureOffset.y);
	glVertex3f(origin.x + vertexCoord, origin.y + vertexCoord, origin.z);

	glTexCoord2f(textureMax + textureOffset.x, textureMax + textureOffset.y);
	glVertex3f(origin.x + vertexCoord, origin.y - vertexCoord, origin.z);

	glEnd();
}

void DrawFloor(const glm::vec3& origin, float floorLength, float textureRepeatLength,
	const glm::vec2& textureOffset, GLuint groundTexture, const glm::vec3& groundColor,
	const bool bMirror)
{
	const auto cullFaceWasEnabled = glIsEnabled(GL_CULL_FACE);
	GLint oldCullFace;

	glGetIntegerv(GL_CULL_FACE_MODE, &oldCullFace);

	glCullFace(GL_FRONT);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	if (bMirror)
	{
		glFrontFace(GL_CW);
	}
	else
	{
		glDisable(GL_CULL_FACE);
	}

	glEnable(GL_BLEND);

	if (groundTexture == GL_INVALID_TEXTURE_ID)
	{
		glDisable(GL_TEXTURE_2D);
		glColor4fv(glm::value_ptr(glm::vec4{groundColor, 0.7}));
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	else
	{
		glEnable(GL_TEXTURE_2D);
		glColor4f(1.0f, 1.0f, 1.0f, 0.6f);
		glBindTexture(GL_TEXTURE_2D, groundTexture);
	}

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	DrawFloorQuad(origin, floorLength, textureRepeatLength, textureOffset);

	glDisable(GL_BLEND);

	if (bMirror)
	{
		glCullFace(GL_BACK);
		glColor4f(0.1f, 0.1f, 0.1f, 1.0f);
		glBindTexture(GL_TEXTURE_2D, 0);
		DrawFloorQuad(origin, floorLength, textureRepeatLength, textureOffset);

		glFrontFace(GL_CCW);
	}

	if (cullFaceWasEnabled)
	{
		glEnable(GL_CULL_FACE);
	}
	else
	{
		glDisable(GL_CULL_FACE);
	}

	glCullFace(static_cast<GLenum>(oldCullFace));
}

unsigned int DrawMirroredModel(studiomdl::IStudioModelRenderer& studioModelRenderer, StudioModelEntity* pEntity,
	const RenderMode renderMode, const bool bWireframeOverlay, const glm::vec3& origin, const float floorLength, const bool bBackfaceCulling)
{
	GLint oldCullFace;

	glGetIntegerv(GL_CULL_FACE_MODE, &oldCullFace);

	/* Don't update color or depth. */
	glDisable(GL_DEPTH_TEST);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

	/* Draw 1 into the stencil buffer. */
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	glStencilFunc(GL_ALWAYS, 1, 0xffffffff);

	//Cull backside of the floor so the model can't draw underneath the floor
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	/* Now render floor; floor pixels just get their stencil set to 1. */
	//Texture length is irrelevant here
	DrawFloorQuad(origin, floorLength, 1, glm::vec2{0});

	/* Re-enable update of color and depth. */
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glEnable(GL_DEPTH_TEST);

	/* Now, only render where stencil is set to 1. */
	glStencilFunc(GL_EQUAL, 1, 0xffffffff);  /* draw if ==1 */
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

	//Note: setting the stencil mask to 0 here would essentially disable the stencil buffer.

	glPushMatrix();
	glScalef(1, 1, -1);
	glCullFace(GL_BACK);
	SetupRenderMode(renderMode, bBackfaceCulling);

	glEnable(GL_CLIP_PLANE0);

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

	glClipPlane(GL_CLIP_PLANE0, flClipPlane);

	const glm::vec3& vecScale = pEntity->GetScale();

	//Determine if an odd number of scale values are negative. The cull face has to be changed if so.
	const float flScale = vecScale.x * vecScale.y * vecScale.z;

	glCullFace(flScale > 0 ? GL_BACK : GL_FRONT);

	const unsigned int uiOldPolys = studioModelRenderer.GetDrawnPolygonsCount();

	renderer::DrawFlags flags = renderer::DrawFlag::NONE;

	//Draw wireframe overlay
	if (bWireframeOverlay)
	{
		flags |= renderer::DrawFlag::WIREFRAME_OVERLAY;
	}

	pEntity->Draw(flags);

	glDisable(GL_CLIP_PLANE0);

	glPopMatrix();

	glDisable(GL_STENCIL_TEST);

	glCullFace(static_cast<GLenum>(oldCullFace));

	return studioModelRenderer.GetDrawnPolygonsCount() - uiOldPolys;
}
}
