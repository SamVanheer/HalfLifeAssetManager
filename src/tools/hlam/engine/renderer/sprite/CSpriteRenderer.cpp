#include <cassert>
#include <cmath>

#include <glm/vec4.hpp>

#include "core/shared/Logging.hpp"
#include "core/shared/WorldTime.hpp"

#include "graphics/OpenGL.hpp"

#include "engine/shared/renderer/sprite/CSpriteRenderInfo.hpp"

#include "engine/shared/sprite/sprite.hpp"

#include "CSpriteRenderer.hpp"

namespace sprite
{
const float CSpriteRenderer::DEFAULT_FRAMERATE = 10;

CSpriteRenderer::CSpriteRenderer(WorldTime* worldTime)
	: _worldTime(worldTime)
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

	assert( pSprite );

	if( !pSprite )
	{
		Error( "CSpriteRenderer::DrawSprite2D: Null sprite!\n" );
		return;
	}

	const auto& framedesc = pSprite->frames[ static_cast<int>( pRenderInfo->flFrame ) ];
	//TODO: get correct frame when it's a group.
	const auto& frame = framedesc.frameptr;

	const sprite::Type::Type* pTypeOverride = pRenderInfo->bOverrideType ? &pRenderInfo->type : nullptr;

	DrawSprite( pRenderInfo->vecOrigin, { frame->width, frame->height }, pSprite, pRenderInfo->flFrame, flags, pTypeOverride );
}

void CSpriteRenderer::DrawSprite2D( const float flX, const float flY, const float flWidth, const float flHeight, const msprite_t* pSprite, const renderer::DrawFlags_t flags )
{
	const float flFrame = static_cast<float>( fmod(_worldTime->GetCurrentTime() * DEFAULT_FRAMERATE, pSprite->numframes ) );

	//TODO: calculate frame
	DrawSprite( { flX, flY, 0 }, { flWidth, flHeight }, pSprite, flFrame, flags );
}

void CSpriteRenderer::DrawSprite2D( const float flX, const float flY, const msprite_t* pSprite, const float flScale, const renderer::DrawFlags_t flags )
{
	assert( pSprite );

	const int iFrame = static_cast<int>( fmod(_worldTime->GetCurrentTime() * DEFAULT_FRAMERATE, pSprite->numframes ) );

	//TODO: calculate frame
	const auto& framedesc = pSprite->frames[ iFrame ];
	
	mspriteframe_t* pFrame;

	if( framedesc.type == spriteframetype_t::SINGLE )
	{
		pFrame = framedesc.frameptr;
	}
	else
	{
		auto pGroup = framedesc.GetGroup();

		//TODO: get correct frame.
		pFrame = pGroup->frames[ 0 ];
	}

	DrawSprite2D( flX, flY, static_cast<float>( pFrame->width * flScale ), static_cast<float>( pFrame->height * flScale ), pSprite, flags );
}

void CSpriteRenderer::DrawSprite2D( const C2DSpriteRenderInfo* pRenderInfo, const renderer::DrawFlags_t flags )
{
	assert( pRenderInfo );

	if( !pRenderInfo )
	{
		Error( "CSpriteRenderer::DrawSprite2D: Null render info!\n" );
		return;
	}

	const auto pSprite = pRenderInfo->pSprite;

	assert( pSprite );

	if( !pSprite )
	{
		Error( "CSpriteRenderer::DrawSprite2D: Null sprite!\n" );
		return;
	}

	const auto& framedesc = pSprite->frames[ static_cast<int>( pRenderInfo->flFrame ) ];

	mspriteframe_t* pFrame;

	if( framedesc.type == spriteframetype_t::SINGLE )
	{
		pFrame = framedesc.frameptr;
	}
	else
	{
		auto pGroup = framedesc.GetGroup();

		//TODO: get correct frame.
		pFrame = pGroup->frames[ 0 ];
	}

	const sprite::TexFormat::TexFormat* pTexFormatOverride = pRenderInfo->bOverrideTexFormat ? &pRenderInfo->texFormat : nullptr;

	DrawSprite( glm::vec3( pRenderInfo->vecPos, 0 ), 
				glm::vec2( pRenderInfo->vecScale.x * pFrame->width, pRenderInfo->vecScale.y * pFrame->height ), 
				pRenderInfo->pSprite, pRenderInfo->flFrame, flags, nullptr, pTexFormatOverride );
}

void CSpriteRenderer::DrawSprite( const glm::vec3& vecOrigin, const glm::vec2& vecSize, 
								  const msprite_t* pSprite, const float flFrame, 
								  const renderer::DrawFlags_t flags, const sprite::Type::Type* pTypeOverride, const sprite::TexFormat::TexFormat* pTexFormatOverride )
{
	assert( pSprite );

	const auto& framedesc = pSprite->frames[ static_cast<int>( floor( flFrame ) ) ];

	mspriteframe_t* pFrame;

	if( framedesc.type == spriteframetype_t::SINGLE )
	{
		pFrame = framedesc.frameptr;
	}
	else
	{
		auto pGroup = framedesc.GetGroup();

		float* pflIntervals = pGroup->intervals;

		double flInt;

		const float flFraction = static_cast<float>( modf( flFrame, &flInt ) );

		int iIndex;

		for( iIndex = 0; iIndex < ( pGroup->numframes - 1 ); ++iIndex )
		{
			if( pflIntervals[ iIndex ] > flFraction )
				break;
		}

		assert( iIndex >= 0 );

		pFrame = pGroup->frames[ iIndex ];
	}

	glEnable( GL_TEXTURE_2D );
	glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
	glBindTexture( GL_TEXTURE_2D, pFrame->gl_texturenum );

	//TODO: set up the sprite's orientation in the world according to its type.
	//TODO: the size of the sprite should change based on its distance from the viewer.

	const sprite::TexFormat::TexFormat texFormat = pTexFormatOverride ? *pTexFormatOverride : pSprite->texFormat;

	switch( texFormat )
	{
	default:
	case TexFormat::SPR_NORMAL:
		{
			glTexEnvi( GL_TEXTURE_2D, GL_TEXTURE_ENV_MODE, GL_MODULATE );
			glDisable( GL_BLEND );
			break;
		}

	case TexFormat::SPR_ADDITIVE:
		{
			glEnable( GL_BLEND );
			glBlendFunc( GL_SRC_ALPHA, GL_ONE );
			break;
		}

	case TexFormat::SPR_INDEXALPHA:
	case TexFormat::SPR_ALPHTEST:
		{
			glEnable( GL_BLEND );
			glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
			break;
		}
	}

	if( texFormat == TexFormat::SPR_ALPHTEST )
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