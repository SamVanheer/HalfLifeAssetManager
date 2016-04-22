/***
*
*	Copyright (c) 1998, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
****/
// updates:
// 1-4-99	fixed file texture load and file read bug

////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/Logging.h"

#include "graphics/OpenGL.h"
#include "graphics/GraphicsHelpers.h"
#include "graphics/Palette.h"

#include "game/CAnimEvent.h"
#include "game/Events.h"
#include "StudioModel.h"



#pragma warning( disable : 4244 ) // double to float

//extern float			g_bonetransform[MAXSTUDIOBONES][3][4];


bool bFilterTextures = true;


////////////////////////////////////////////////////////////////////////

StudioModel::StudioModel()
{
	memset( m_Textures, 0, sizeof( m_Textures ) );
}

StudioModel::~StudioModel()
{
	FreeModel();
}

void StudioModel::UploadTexture(mstudiotexture_t *ptexture, byte *data, byte *pal, int name)
{
	// unsigned *in, int inwidth, int inheight, unsigned *out,  int outwidth, int outheight;
	int		i, j;
	int		row1[MAX_TEXTURE_DIMS], row2[MAX_TEXTURE_DIMS], col1[MAX_TEXTURE_DIMS], col2[MAX_TEXTURE_DIMS];
	byte	*pix1, *pix2, *pix3, *pix4;
	byte	*tex, *out;

	// convert texture to power of 2
	int outwidth;
	int outheight;

	if( !graphics::helpers::CalculateImageDimensions( ptexture->width, ptexture->height, outwidth, outheight ) )
		return;

	tex = out = (byte *)malloc( outwidth * outheight * 4);
	if (!out)
	{
		return;
	}
/*
	int k = 0;
	for (i = 0; i < ptexture->height; i++)
	{
		for (j = 0; j < ptexture->width; j++)
		{

			in[k++] = pal[data[i * ptexture->width + j] * 3 + 0];
			in[k++] = pal[data[i * ptexture->width + j] * 3 + 1];
			in[k++] = pal[data[i * ptexture->width + j] * 3 + 2];
			in[k++] = 0xff;;
		}
	}

	gluScaleImage (GL_RGBA, ptexture->width, ptexture->height, GL_UNSIGNED_BYTE, in, outwidth, outheight, GL_UNSIGNED_BYTE, out);
	free (in);
*/

	for (i = 0; i < outwidth; i++)
	{
		col1[i] = (int) ((i + 0.25) * (ptexture->width / (float)outwidth));
		col2[i] = (int) ((i + 0.75) * (ptexture->width / (float)outwidth));
	}

	for (i = 0; i < outheight; i++)
	{
		row1[i] = (int) ((i + 0.25) * (ptexture->height / (float)outheight)) * ptexture->width;
		row2[i] = (int) ((i + 0.75) * (ptexture->height / (float)outheight)) * ptexture->width;
	}

	const byte* const pAlpha = &pal[ PALETTE_ALPHA_INDEX ];

	// scale down and convert to 32bit RGB
	for (i=0 ; i<outheight ; i++)
	{
		for (j=0 ; j<outwidth ; j++, out += 4)
		{
			pix1 = &pal[data[row1[i] + col1[j]] * 3];
			pix2 = &pal[data[row1[i] + col2[j]] * 3];
			pix3 = &pal[data[row2[i] + col1[j]] * 3];
			pix4 = &pal[data[row2[i] + col2[j]] * 3];

			out[0] = (pix1[0] + pix2[0] + pix3[0] + pix4[0])>>2;
			out[1] = (pix1[1] + pix2[1] + pix3[1] + pix4[1])>>2;
			out[2] = (pix1[2] + pix2[2] + pix3[2] + pix4[2])>>2;

			if( ptexture->flags & STUDIO_NF_MASKED && pix1 == pAlpha && pix2 == pAlpha && pix3 == pAlpha && pix4 == pAlpha )
			{
				out[ 3 ] = 0x00;
			}
			else
			{
				out[3] = 0xFF;
			}
		}
	}

	UploadRGBATexture( outwidth, outheight, tex, name );

	free( tex );
}

void StudioModel::UploadRGBATexture( const int iWidth, const int iHeight, byte* pData, GLuint textureId )
{
	glBindTexture( GL_TEXTURE_2D, textureId );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, iWidth, iHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, pData );
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, bFilterTextures ? GL_LINEAR : GL_NEAREST );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, bFilterTextures ? GL_LINEAR : GL_NEAREST );
}

void StudioModel::ReplaceTexture( mstudiotexture_t *ptexture, byte *data, byte *pal, GLuint textureId )
{
	glDeleteTextures( 1, &textureId );

	UploadTexture( ptexture, data, pal, textureId );
}

void
StudioModel::FreeModel ()
{
	if( !m_pstudiohdr )
		return;

	m_TextureMeshMap.clear();

	// deleting textures
	glDeleteTextures( m_ptexturehdr->numtextures, m_Textures );

	for( int i = 0; i < 32; i++ )
	{
		if( m_panimhdr[ i ] )
		{
			free( m_panimhdr[ i ] );
			m_panimhdr[ i ] = nullptr;
		}
	}

	if( m_ptexturehdr && m_owntexmodel )
		free( m_ptexturehdr );

	if (m_pstudiohdr)
		free (m_pstudiohdr);

	m_pstudiohdr = m_ptexturehdr = nullptr;
	m_owntexmodel = false;

	memset( m_Textures, 0, sizeof( m_Textures ) );
}



studiohdr_t *StudioModel::LoadModel( const char* pszModelName, LoadResult* pResult )
{
	if( pResult )
		*pResult = LoadResult::FAILURE;

	FILE *fp;
	long size;
	void *buffer;

	if( !pszModelName )
		return nullptr;

	// load the model
	if( (fp = fopen( pszModelName, "rb" )) == nullptr)
		return nullptr;

	fseek( fp, 0, SEEK_END );
	size = ftell( fp );
	fseek( fp, 0, SEEK_SET );

	buffer = malloc( size );
	if (!buffer)
	{
		fclose (fp);
		return nullptr;
	}

	fread( buffer, size, 1, fp );
	fclose( fp );

	byte				*pin;
	studiohdr_t			*phdr;
	mstudiotexture_t	*ptexture;

	pin = (byte *)buffer;
	phdr = (studiohdr_t *)pin;
	ptexture = (mstudiotexture_t *)(pin + phdr->textureindex);

	if (strncmp ((const char *) buffer, "IDST", 4) &&
		strncmp ((const char *) buffer, "IDSQ", 4))
	{
		free (buffer);
		return nullptr;
	}

	if (!strncmp ((const char *) buffer, "IDSQ", 4) && !m_pstudiohdr)
	{
		free (buffer);
		return nullptr;
	}

	if( phdr->version != STUDIO_VERSION )
	{
		if( pResult )
			*pResult = LoadResult::VERSIONDIFFERS;

		free( buffer );
		return nullptr;
	}

	if (phdr->textureindex > 0 && phdr->numtextures <= MAXSTUDIOSKINS)
	{
		int n = phdr->numtextures;
		for (int i = 0; i < n; i++)
		{
			GLuint name;

			glBindTexture( GL_TEXTURE_2D, 0 );
			glGenTextures( 1, &name );

			UploadTexture( &ptexture[i], pin + ptexture[i].index, pin + ptexture[i].width * ptexture[i].height + ptexture[i].index, name );

			m_Textures[ i ] = name;
		}
	}

	// UNDONE: free texture memory

	if (!m_pstudiohdr)
		m_pstudiohdr = (studiohdr_t *)buffer;

	if( pResult )
		*pResult = LoadResult::SUCCESS;

	return (studiohdr_t *)buffer;
}



bool StudioModel::PostLoadModel( const char* const pszModelName )
{
	// preload textures
	if (m_pstudiohdr->numtextures == 0)
	{
		char texturename[256];

		strcpy( texturename, pszModelName );
		strcpy( &texturename[strlen(texturename) - 4], "T.mdl" );

		m_ptexturehdr = LoadModel( texturename );
		if (!m_ptexturehdr)
		{
			FreeModel ();
			return false;
		}
		m_owntexmodel = true;
	}
	else
	{
		m_ptexturehdr = m_pstudiohdr;
		m_owntexmodel = false;
	}

	// preload animations
	if (m_pstudiohdr->numseqgroups > 1)
	{
		for (int i = 1; i < m_pstudiohdr->numseqgroups; i++)
		{
			char seqgroupname[256];

			strcpy( seqgroupname, pszModelName );
			sprintf( &seqgroupname[strlen(seqgroupname) - 4], "%02d.mdl", i );

			m_panimhdr[i] = LoadModel( seqgroupname );
			if (!m_panimhdr[i])
			{
				FreeModel ();
				return false;
			}
		}
	}

	SetSequence (0);
	SetController (0, 0.0f);
	SetController (1, 0.0f);
	SetController (2, 0.0f);
	SetController (3, 0.0f);
	SetMouth (0.0f);

	int n;
	for (n = 0; n < m_pstudiohdr->numbodyparts; n++)
		SetBodygroup (n, 0);

	SetSkin (0);

	//Build texture->meshes map.
	m_TextureMeshMap.resize( m_ptexturehdr->numtextures );

	const short* const pskinref = ( const short* ) ( ( const byte* ) m_ptexturehdr + m_ptexturehdr->skinindex );

	for( int iBodyPart = 0; iBodyPart < m_pstudiohdr->numbodyparts; ++iBodyPart )
	{
		const mstudiomodel_t* const pModel = GetModelByBodyPart( iBodyPart );

		for( int iMesh = 0; iMesh < pModel->nummesh; ++iMesh )
		{
			const mstudiomesh_t* pMesh = ( ( const mstudiomesh_t* ) ( ( const byte* ) m_pstudiohdr + pModel->meshindex ) ) + iMesh;

			m_TextureMeshMap[ pskinref[ pMesh->skinref ] ].push_back( pMesh );
		}
	}

/*
	glm::vec3 mins, maxs;
	ExtractBbox (mins, maxs);
	if (mins[2] < 5.0f)
		m_origin[2] = -mins[2];
*/
	return true;
}

StudioModel::LoadResult StudioModel::Load( const char* const pszModelName )
{
	if( !pszModelName || !( *pszModelName ) )
		return LoadResult::FAILURE;

	LoadResult result = LoadResult::FAILURE;

	LoadModel( pszModelName, &result );

	if( result != LoadResult::SUCCESS )
	{
		return result;
	}

	if( !PostLoadModel( pszModelName ) )
	{
		return LoadResult::POSTLOADFAILURE;
	}

	m_flFrameRate = 1.0f;

	return LoadResult::SUCCESS;
}

bool StudioModel::SaveModel ( char *modelname )
{
	if (!modelname)
		return false;

	if (!m_pstudiohdr)
		return false;

	FILE *file;
	
	file = fopen (modelname, "wb");
	if (!file)
		return false;

	fwrite (m_pstudiohdr, sizeof (byte), m_pstudiohdr->length, file);
	fclose (file);

	// write texture model
	if (m_owntexmodel && m_ptexturehdr)
	{
		char texturename[256];

		strcpy( texturename, modelname );
		strcpy( &texturename[strlen(texturename) - 4], "T.mdl" );

		file = fopen (texturename, "wb");
		if (file)
		{
			fwrite (m_ptexturehdr, sizeof (byte), m_ptexturehdr->length, file);
			fclose (file);
		}
	}

	// write seq groups
	if (m_pstudiohdr->numseqgroups > 1)
	{
		for (int i = 1; i < m_pstudiohdr->numseqgroups; i++)
		{
			char seqgroupname[256];

			strcpy( seqgroupname, modelname );
			sprintf( &seqgroupname[strlen(seqgroupname) - 4], "%02d.mdl", i );

			file = fopen (seqgroupname, "wb");
			if (file)
			{
				fwrite (m_panimhdr[i], sizeof (byte), m_panimhdr[i]->length, file);
				fclose (file);
			}
		}
	}

	return true;
}



////////////////////////////////////////////////////////////////////////

int StudioModel::GetSequence( )
{
	return m_sequence;
}

int StudioModel::SetSequence( int iSequence )
{
	if (iSequence > m_pstudiohdr->numseq)
		return m_sequence;

	m_sequence = iSequence;
	m_frame = 0;

	return m_sequence;
}

void StudioModel::GetSequenceInfo( float *pflFrameRate, float *pflGroundSpeed ) const
{
	const mstudioseqdesc_t* pseqdesc = m_pstudiohdr->GetSequence( m_sequence );

	if (pseqdesc->numframes > 1)
	{
		*pflFrameRate = 256 * pseqdesc->fps / (pseqdesc->numframes - 1);
		*pflGroundSpeed = sqrt( pseqdesc->linearmovement[0]*pseqdesc->linearmovement[0]+ 
								pseqdesc->linearmovement[1]*pseqdesc->linearmovement[1]+ 
								pseqdesc->linearmovement[2]*pseqdesc->linearmovement[2] );
		*pflGroundSpeed = *pflGroundSpeed * pseqdesc->fps / (pseqdesc->numframes - 1);
	}
	else
	{
		*pflFrameRate = 256.0;
		*pflGroundSpeed = 0.0;
	}
}

void StudioModel::ExtractBbox( glm::vec3& vecMins, glm::vec3& vecMaxs ) const
{
	const mstudioseqdesc_t* pseqdesc = m_pstudiohdr->GetSequence( m_sequence );

	vecMins[ 0 ] = pseqdesc->bbmin[ 0 ];
	vecMins[ 1 ] = pseqdesc->bbmin[ 1 ];
	vecMins[ 2 ] = pseqdesc->bbmin[ 2 ];

	vecMaxs[ 0 ] = pseqdesc->bbmax[ 0 ];
	vecMaxs[ 1 ] = pseqdesc->bbmax[ 1 ];
	vecMaxs[ 2 ] = pseqdesc->bbmax[ 2 ];
}

byte StudioModel::GetBoneController( int iController ) const
{
	return m_controller[ iController ];
}

float StudioModel::GetController( int iController ) const
{
	if( !m_pstudiohdr )
		return 0.0f;

	if( iController < 0 || iController > CONTROLLER_MOUTH_INDEX )
		return 0;

	const mstudiobonecontroller_t* pbonecontroller = ( mstudiobonecontroller_t* ) ( ( byte* ) m_pstudiohdr + m_pstudiohdr->bonecontrollerindex );

	// find first controller that matches the index
	int i;
	for( i = 0; i < m_pstudiohdr->numbonecontrollers; i++, pbonecontroller++ )
	{
		if( pbonecontroller->index == iController )
			break;
	}
	if( i >= m_pstudiohdr->numbonecontrollers )
		return 0;

	byte uiValue;

	if( iController == CONTROLLER_MOUTH_INDEX )
		uiValue = m_mouth;
	else
		uiValue = m_controller[ iController ];

	return uiValue * ( 1.0 / 255.0 ) * ( pbonecontroller->end - pbonecontroller->start ) + pbonecontroller->start;
}

float StudioModel::SetController( int iController, float flValue )
{
	if (!m_pstudiohdr)
		return 0.0f;

	mstudiobonecontroller_t	*pbonecontroller = (mstudiobonecontroller_t *)((byte *)m_pstudiohdr + m_pstudiohdr->bonecontrollerindex);

	// find first controller that matches the index
	int i;
	for (i = 0; i < m_pstudiohdr->numbonecontrollers; i++, pbonecontroller++)
	{
		if (pbonecontroller->index == iController)
			break;
	}
	if (i >= m_pstudiohdr->numbonecontrollers)
		return flValue;

	// wrap 0..360 if it's a rotational controller
	if (pbonecontroller->type & (STUDIO_XR | STUDIO_YR | STUDIO_ZR))
	{
		// ugly hack, invert value if end < start
		if (pbonecontroller->end < pbonecontroller->start)
			flValue = -flValue;

		// does the controller not wrap?
		if (pbonecontroller->start + 359.0 >= pbonecontroller->end)
		{
			if (flValue > ((pbonecontroller->start + pbonecontroller->end) / 2.0) + 180)
				flValue = flValue - 360;
			if (flValue < ((pbonecontroller->start + pbonecontroller->end) / 2.0) - 180)
				flValue = flValue + 360;
		}
		else
		{
			if (flValue > 360)
				flValue = flValue - (int)(flValue / 360.0) * 360.0;
			else if (flValue < 0)
				flValue = flValue + (int)((flValue / -360.0) + 1) * 360.0;
		}
	}

	int setting = (int) (255 * (flValue - pbonecontroller->start) /
	(pbonecontroller->end - pbonecontroller->start));

	if (setting < 0) setting = 0;
	if (setting > 255) setting = 255;
	m_controller[iController] = setting;

	return setting * (1.0 / 255.0) * (pbonecontroller->end - pbonecontroller->start) + pbonecontroller->start;
}


float StudioModel::SetMouth( float flValue )
{
	if (!m_pstudiohdr)
		return 0.0f;

	mstudiobonecontroller_t	*pbonecontroller = (mstudiobonecontroller_t *)((byte *)m_pstudiohdr + m_pstudiohdr->bonecontrollerindex);

	// find first controller that matches the mouth
	for (int i = 0; i < m_pstudiohdr->numbonecontrollers; i++, pbonecontroller++)
	{
		if (pbonecontroller->index == CONTROLLER_MOUTH_INDEX )
			break;
	}

	// wrap 0..360 if it's a rotational controller
	if (pbonecontroller->type & (STUDIO_XR | STUDIO_YR | STUDIO_ZR))
	{
		// ugly hack, invert value if end < start
		if (pbonecontroller->end < pbonecontroller->start)
			flValue = -flValue;

		// does the controller not wrap?
		if (pbonecontroller->start + 359.0 >= pbonecontroller->end)
		{
			if (flValue > ((pbonecontroller->start + pbonecontroller->end) / 2.0) + 180)
				flValue = flValue - 360;
			if (flValue < ((pbonecontroller->start + pbonecontroller->end) / 2.0) - 180)
				flValue = flValue + 360;
		}
		else
		{
			if (flValue > 360)
				flValue = flValue - (int)(flValue / 360.0) * 360.0;
			else if (flValue < 0)
				flValue = flValue + (int)((flValue / -360.0) + 1) * 360.0;
		}
	}

	int setting = (int) (64 * (flValue - pbonecontroller->start) / (pbonecontroller->end - pbonecontroller->start));

	if (setting < 0) setting = 0;
	if (setting > 64) setting = 64;
	m_mouth = setting;

	return setting * (1.0 / 64.0) * (pbonecontroller->end - pbonecontroller->start) + pbonecontroller->start;
}


float StudioModel::SetBlending( int iBlender, float flValue )
{
	mstudioseqdesc_t	*pseqdesc;

	if (!m_pstudiohdr)
		return 0.0f;

	pseqdesc = (mstudioseqdesc_t *)((byte *)m_pstudiohdr + m_pstudiohdr->seqindex) + (int)m_sequence;

	if (pseqdesc->blendtype[iBlender] == 0)
		return flValue;

	if (pseqdesc->blendtype[iBlender] & (STUDIO_XR | STUDIO_YR | STUDIO_ZR))
	{
		// ugly hack, invert value if end < start
		if (pseqdesc->blendend[iBlender] < pseqdesc->blendstart[iBlender])
			flValue = -flValue;

		// does the controller not wrap?
		if (pseqdesc->blendstart[iBlender] + 359.0 >= pseqdesc->blendend[iBlender])
		{
			if (flValue > ((pseqdesc->blendstart[iBlender] + pseqdesc->blendend[iBlender]) / 2.0) + 180)
				flValue = flValue - 360;
			if (flValue < ((pseqdesc->blendstart[iBlender] + pseqdesc->blendend[iBlender]) / 2.0) - 180)
				flValue = flValue + 360;
		}
	}

	int setting = (int) (255 * (flValue - pseqdesc->blendstart[iBlender]) / (pseqdesc->blendend[iBlender] - pseqdesc->blendstart[iBlender]));

	if (setting < 0) setting = 0;
	if (setting > 255) setting = 255;

	m_blending[iBlender] = setting;

	return setting * (1.0 / 255.0) * (pseqdesc->blendend[iBlender] - pseqdesc->blendstart[iBlender]) + pseqdesc->blendstart[iBlender];
}



int StudioModel::SetBodygroup( int iGroup, int iValue )
{
	if (!m_pstudiohdr)
		return 0;

	if (iGroup > m_pstudiohdr->numbodyparts)
		return -1;

	mstudiobodyparts_t *pbodypart = (mstudiobodyparts_t *)((byte *)m_pstudiohdr + m_pstudiohdr->bodypartindex) + iGroup;

	int iCurrent = (m_bodynum / pbodypart->base) % pbodypart->nummodels;

	if (iValue >= pbodypart->nummodels)
		return iCurrent;

	m_bodynum = (m_bodynum - (iCurrent * pbodypart->base) + (iValue * pbodypart->base));

	return iValue;
}


int StudioModel::SetSkin( int iValue )
{
	if (!m_pstudiohdr)
		return 0;

	if (iValue >= m_pstudiohdr->numskinfamilies)
	{
		return m_skinnum;
	}

	m_skinnum = iValue;

	return iValue;
}

int StudioModel::GetAnimationEvent( CAnimEvent& event, float flStart, float flEnd, int index, const bool bAllowClientEvents )
{
	if( !m_pstudiohdr || m_sequence >= m_pstudiohdr->numseq )
		return 0;

	int events = 0;

	const mstudioseqdesc_t* pseqdesc = ( mstudioseqdesc_t * ) ( ( byte * ) m_pstudiohdr + m_pstudiohdr->seqindex ) + m_sequence;
	const mstudioevent_t* pevent = ( mstudioevent_t * ) ( ( byte * ) m_pstudiohdr + pseqdesc->eventindex );

	if( pseqdesc->numevents == 0 || index > pseqdesc->numevents )
		return 0;

	if( pseqdesc->numframes <= 1 )
	{
		flStart = 0;
		flEnd = 1.0;
	}

	for( ; index < pseqdesc->numevents; index++ )
	{
		//TODO: maybe leave it up to the listener to filter these out?
		if( !bAllowClientEvents )
		{
			// Don't send client-side events to the server AI
			if( pevent[ index ].event >= EVENT_CLIENT )
				continue;
		}

		if( ( pevent[ index ].frame >= flStart && pevent[ index ].frame < flEnd ) ||
			( ( pseqdesc->flags & STUDIO_LOOPING ) && flEnd >= pseqdesc->numframes - 1 && pevent[ index ].frame < flEnd - pseqdesc->numframes + 1 ) )
		{
			event.iEvent = pevent[ index ].event;
			event.pszOptions = pevent[ index ].options;
			return index + 1;
		}
	}
	return 0;
}

void StudioModel::DispatchAnimEvents( IAnimEventHandler& handler, const bool bAllowClientEvents, float flInterval )
{
	if( !m_pstudiohdr )
	{
		Message( "Gibbed monster is thinking!\n" );
		return;
	}

	// FIXME: I have to do this or some events get missed, and this is probably causing the problem below
	//This isn't really necessary, at least not in a tool.
	//flInterval = 0.1f;

	const mstudioseqdesc_t* pseqdesc = ( mstudioseqdesc_t * ) ( ( byte * ) m_pstudiohdr + m_pstudiohdr->seqindex ) + m_sequence;

	// FIX: this still sometimes hits events twice
	float flStart = m_frame + ( m_flLastEventCheck - m_flAnimTime ) * pseqdesc->fps * m_flFrameRate;
	float flEnd = m_frame + flInterval * pseqdesc->fps * m_flFrameRate;
	m_flLastEventCheck = m_flAnimTime + flInterval;

	CAnimEvent event;

	int index = 0;

	while( ( index = GetAnimationEvent( event, flStart, flEnd, index, bAllowClientEvents ) ) != 0 )
	{
		handler.HandleAnimEvent( event );
	}
}

void StudioModel::ScaleMeshes( float scale )
{
	if( !m_pstudiohdr )
		return;

	int i, j, k;

	// scale verts
	int tmp = m_bodynum;
	for( i = 0; i < m_pstudiohdr->numbodyparts; i++ )
	{
		mstudiobodyparts_t *pbodypart = ( mstudiobodyparts_t * ) ( ( byte * ) m_pstudiohdr + m_pstudiohdr->bodypartindex ) + i;
		for( j = 0; j < pbodypart->nummodels; j++ )
		{
			SetBodygroup( i, j );
			SetupModel( i );

			glm::vec3 *pstudioverts = ( glm::vec3 * )( ( byte * ) m_pstudiohdr + m_pmodel->vertindex );

			for( k = 0; k < m_pmodel->numverts; k++ )
			{
				pstudioverts[ k ] *= scale;
			}
		}
	}

	m_bodynum = tmp;

	// scale complex hitboxes
	mstudiobbox_t *pbboxes = ( mstudiobbox_t * ) ( ( byte * ) m_pstudiohdr + m_pstudiohdr->hitboxindex );
	for( i = 0; i < m_pstudiohdr->numhitboxes; i++ )
	{
		pbboxes[ i ].bbmin *= scale;
		pbboxes[ i ].bbmax *= scale;
	}

	// scale bounding boxes
	mstudioseqdesc_t *pseqdesc = ( mstudioseqdesc_t * ) ( ( byte * ) m_pstudiohdr + m_pstudiohdr->seqindex );
	for( i = 0; i < m_pstudiohdr->numseq; i++ )
	{
		pseqdesc[ i ].bbmin *= scale;
		pseqdesc[ i ].bbmax *= scale;
	}

	// maybe scale exeposition, pivots, attachments
}



void StudioModel::ScaleBones( float scale )
{
	if( !m_pstudiohdr )
		return;

	mstudiobone_t *pbones = ( mstudiobone_t * ) ( ( byte * ) m_pstudiohdr + m_pstudiohdr->boneindex );
	for( int i = 0; i < m_pstudiohdr->numbones; i++ )
	{
		for( int j = 0; j < 3; j++ )
		{
			pbones[ i ].value[ j ] *= scale;
			pbones[ i ].scale[ j ] *= scale;
		}
	}
}