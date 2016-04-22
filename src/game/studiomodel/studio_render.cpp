/***
*
*	Copyright (c) 1998, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
****/
// studio_render.cpp: routines for drawing Half-Life 3DStudio models
// updates:
// 1-4-99	fixed AdvanceFrame wraping bug

#include <memory>
#include <algorithm>

#include <glm/gtc/matrix_transform.hpp>
//TODO: avoid using value_ptr unless it's absolutely necessary
#include <glm/gtc/type_ptr.hpp>

#include "graphics/OpenGL.h"

#include "common/Logging.h"
#include "common/CGlobals.h"
#include "common/Utility.h"

#include "game/CAnimEvent.h"

#include "cvar/CCVar.h"
#include "cvar/CVarUtils.h"

#include "StudioModel.h"

#pragma warning( disable : 4244 ) // double to float

////////////////////////////////////////////////////////////////////////

mstudioanim_t * StudioModel::GetAnim( mstudioseqdesc_t *pseqdesc )
{
	mstudioseqgroup_t	*pseqgroup;
	pseqgroup = (mstudioseqgroup_t *)((byte *)m_pstudiohdr + m_pstudiohdr->seqgroupindex) + pseqdesc->seqgroup;

	if (pseqdesc->seqgroup == 0)
	{
		return (mstudioanim_t *)((byte *)m_pstudiohdr + pseqgroup->unused2 + pseqdesc->animindex);
	}

	return (mstudioanim_t *)((byte *)m_panimhdr[pseqdesc->seqgroup] + pseqdesc->animindex);
}

GLuint StudioModel::GetTextureId( const int iIndex ) const
{
	const studiohdr_t* const pHdr = GetTextureHeader();

	if( !pHdr )
		return GL_INVALID_TEXTURE_ID;

	if( iIndex < 0 || iIndex >= pHdr->numtextures )
		return GL_INVALID_TEXTURE_ID;

	return m_Textures[ iIndex ];
}

int StudioModel::GetNumFrames() const
{
	const mstudioseqdesc_t* const pseqdesc = ( mstudioseqdesc_t* ) ( ( byte* ) m_pstudiohdr + m_pstudiohdr->seqindex ) + m_sequence;

	return pseqdesc->numframes;
}

float StudioModel::AdvanceFrame( float dt )
{
	if( !m_pstudiohdr )
		return 0.0;

	mstudioseqdesc_t	*pseqdesc = ( mstudioseqdesc_t * ) ( ( byte * ) m_pstudiohdr + m_pstudiohdr->seqindex ) + m_sequence;

	if( dt == 0.0 )
	{
		dt = ( Globals.GetCurrentTime() - m_flAnimTime );
		if( dt <= 0.001 )
		{
			m_flAnimTime = Globals.GetCurrentTime();
			return 0.0;
		}
	}

	if( !m_flAnimTime )
		dt = 0.0;

	/*
	if( dt > 0.1f )
	dt = 0.1f;
	*/

	m_frame += dt * pseqdesc->fps * m_flFrameRate;

	if( pseqdesc->numframes <= 1 )
	{
		m_frame = 0;
	}
	else
	{
		// wrap
		m_frame -= ( int ) ( m_frame / ( pseqdesc->numframes - 1 ) ) * ( pseqdesc->numframes - 1 );
	}

	m_flAnimTime = Globals.GetCurrentTime();

	return dt;
}

int StudioModel::SetFrame( int nFrame )
{
	if( nFrame == -1 )
		return m_frame;

	if( !m_pstudiohdr )
		return 0;

	mstudioseqdesc_t	*pseqdesc;
	pseqdesc = ( mstudioseqdesc_t * ) ( ( byte * ) m_pstudiohdr + m_pstudiohdr->seqindex ) + m_sequence;

	m_frame = nFrame;

	if( pseqdesc->numframes <= 1 )
	{
		m_frame = 0;
	}
	else
	{
		// wrap
		m_frame -= ( int ) ( m_frame / ( pseqdesc->numframes - 1 ) ) * ( pseqdesc->numframes - 1 );
	}

	m_flAnimTime = Globals.GetCurrentTime();

	return m_frame;
}

mstudiomodel_t* StudioModel::GetModelByBodyPart( const int iBodyPart ) const
{
	mstudiobodyparts_t* pbodypart = ( mstudiobodyparts_t* ) ( ( byte* ) m_pstudiohdr + m_pstudiohdr->bodypartindex ) + iBodyPart;

	int index = m_bodynum / pbodypart->base;
	index = index % pbodypart->nummodels;

	return ( mstudiomodel_t * ) ( ( byte * ) m_pstudiohdr + pbodypart->modelindex ) + index;
}

const StudioModel::MeshList_t* StudioModel::GetMeshListByTexture( const int iIndex ) const
{
	if( !m_ptexturehdr )
		return nullptr;

	if( iIndex < 0 || iIndex >= m_ptexturehdr->numtextures )
		return nullptr;

	return &m_TextureMeshMap[ iIndex ];
}