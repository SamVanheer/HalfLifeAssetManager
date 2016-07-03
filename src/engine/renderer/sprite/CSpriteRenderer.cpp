#include <cassert>

#include <glm/vec4.hpp>

#include "core/shared/Logging.h"

#include "graphics/OpenGL.h"

#include "shared/CWorldTime.h"

#include "engine/shared/renderer/sprite/CSpriteRenderInfo.h"

#include "engine/shared/sprite/sprite.h"

#include "CSpriteRenderer.h"

namespace sprite
{
REGISTER_SINGLE_INTERFACE( ISPRITERENDERER_NAME, CSpriteRenderer );

CSpriteRenderer::CSpriteRenderer()
{
}

CSpriteRenderer::~CSpriteRenderer()
{
}

void CSpriteRenderer::DrawSprite( const CSpriteRenderInfo* pRenderInfo, const renderer::DrawFlags_t flags )
{
	assert( pRenderInfo );

	if( !pRenderInfo )
	{
		Error( "CSpriteRenderer::DrawSprite: Null render info!\n" );
		return;
	}

	const auto pSprite = pRenderInfo->pSprite;

	const auto& framedesc = pSprite->frames[ static_cast<int>( pRenderInfo->flFrame ) ];
	const auto& frame = framedesc.frameptr;

	DrawSprite( pRenderInfo->vecOrigin, { frame->width, frame->height }, pSprite, static_cast<int>( pRenderInfo->flFrame ), flags );
}

void CSpriteRenderer::DrawSprite2D( const float flX, const float flY, const float flWidth, const float flHeight, const msprite_t* pSprite, const renderer::DrawFlags_t flags )
{
	const int iFrame = static_cast<int>( fmod( WorldTime.GetCurrentTime() * 10, pSprite->numframes ) );

	//TODO: calculate frame
	DrawSprite( { flX, flY, 0 }, { flWidth, flHeight }, pSprite, iFrame, flags );
}

void CSpriteRenderer::DrawSprite2D( const float flX, const float flY, const msprite_t* pSprite, const float flScale, const renderer::DrawFlags_t flags )
{
	assert( pSprite );

	const int iFrame = static_cast<int>( fmod( WorldTime.GetCurrentTime() * 10, pSprite->numframes ) );

	//TODO: calculate frame
	const auto& framedesc = pSprite->frames[ iFrame ];
	const auto& frame = framedesc.frameptr;

	DrawSprite2D( flX, flY, static_cast<float>( frame->width * flScale ), static_cast<float>( frame->height * flScale ), pSprite, flags );
}

void CSpriteRenderer::DrawSprite( const glm::vec3& vecOrigin, const glm::vec2& vecSize, const msprite_t* pSprite, const int iFrame, const renderer::DrawFlags_t flags )
{
	assert( pSprite );

	const auto& framedesc = pSprite->frames[ iFrame ];
	const auto& frame = framedesc.frameptr;

	glEnable( GL_TEXTURE_2D );
	glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
	glBindTexture( GL_TEXTURE_2D, frame->gl_texturenum );

	//TODO: set up the sprite's orientation in the world according to its type.
	//TODO: the size of the sprite should change based on its distance from the viewer.

	switch( pSprite->texFormat )
	{
	default:
	case TexFormat_t::SPR_NORMAL:
	case TexFormat_t::SPR_ADDITIVE:
		{
			glEnable( GL_BLEND );
			glBlendFunc( GL_SRC_ALPHA, GL_ONE );
			break;
		}

	case TexFormat_t::SPR_INDEXALPHA:
	case TexFormat_t::SPR_ALPHTEST:
		{
			glEnable( GL_BLEND );
			glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
			break;
		}
	}

	if( pSprite->texFormat == TexFormat_t::SPR_ALPHTEST )
	{
		glEnable( GL_ALPHA_TEST );
		glAlphaFunc( GL_GREATER, 0.0f );
	}
	else
	{
		glDisable( GL_ALPHA_TEST );
	}

	const glm::vec4 vecRect{ vecOrigin.x - vecSize.x / 2, vecOrigin.y - vecSize.y / 2, vecOrigin.x + vecSize.x / 2, vecOrigin.y + vecSize.y / 2 };

	if( !( flags & renderer::DrawFlag::NODRAW ) )
	{
		glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
		glEnable( GL_TEXTURE_2D );
		glEnable( GL_CULL_FACE );
		glEnable( GL_DEPTH_TEST );
		glShadeModel( GL_SMOOTH );
		glColor4f( 1, 1, 1, 1 );

		glBegin( GL_TRIANGLE_STRIP );

		glTexCoord2f( 0, 0 );
		glVertex3f( vecRect.x, vecRect.y, vecOrigin.z );

		glTexCoord2f( 1, 0 );
		glVertex3f( vecRect.z, vecRect.y, vecOrigin.z );

		glTexCoord2f( 0, 1 );
		glVertex3f( vecRect.x, vecRect.w, vecOrigin.z );

		glTexCoord2f( 1, 1 );
		glVertex3f( vecRect.z, vecRect.w, vecOrigin.z );

		glEnd();
	}

	if( flags & renderer::DrawFlag::WIREFRAME_OVERLAY )
	{
		glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		glDisable( GL_TEXTURE_2D );
		glDisable( GL_CULL_FACE );
		glDisable( GL_DEPTH_TEST );
		glColor4f( 1, 1, 1, 1 );

		glBegin( GL_TRIANGLE_STRIP );

		glVertex3f( vecRect.x, vecRect.y, vecOrigin.z );

		glVertex3f( vecRect.z, vecRect.y, vecOrigin.z );

		glVertex3f( vecRect.x, vecRect.w, vecOrigin.z );

		glVertex3f( vecRect.z, vecRect.w, vecOrigin.z );

		glEnd();
	}
}
}