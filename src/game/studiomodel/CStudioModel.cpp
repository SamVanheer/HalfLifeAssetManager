#include <cassert>
#include <memory>

#include "graphics/GraphicsHelpers.h"
#include "graphics/Palette.h"

#include "CStudioModel.h"

namespace studiomodel
{
CStudioModel::CStudioModel()
{
	memset( this, 0, sizeof( *this ) );
}

CStudioModel::CStudioModel( studiohdr_t* pStudioHdr, studiohdr_t* pTextureHdr, studiohdr_t** ppSeqHdrs, const size_t uiNumSeqHdrs, GLuint* pTextures, const size_t uiNumTextures )
	: m_pStudioHdr( pStudioHdr )
	, m_pTextureHdr( pTextureHdr )
{
	assert( pStudioHdr );
	assert( pTextureHdr );
	assert( ppSeqHdrs );
	assert( uiNumSeqHdrs <= MAX_SEQGROUPS );
	assert( pTextures );
	assert( uiNumTextures <= MAX_TEXTURES );

	for( size_t uiIndex = 0; uiIndex < uiNumSeqHdrs; ++uiIndex )
	{
		m_pSeqHdrs[ uiIndex ] = ppSeqHdrs[ uiIndex ];
	}

	memset( m_pSeqHdrs + uiNumSeqHdrs, 0, sizeof( studiohdr_t* ) * MAX_SEQGROUPS - uiNumSeqHdrs );

	memcpy( m_Textures, pTextureHdr, uiNumTextures );
	memset( m_Textures + uiNumTextures, 0, sizeof( GLuint ) * MAX_TEXTURES - uiNumTextures );
}

CStudioModel::~CStudioModel()
{
	if( !m_pStudioHdr )
		return;

	// deleting textures
	glDeleteTextures( m_pTextureHdr->numtextures, m_Textures );

	for( auto pSeqHdr : m_pSeqHdrs )
	{
		delete[] pSeqHdr;
	}

	//Textures were in a T.mdl, free separately.
	if( m_pTextureHdr != m_pStudioHdr )
	{
		delete[] m_pTextureHdr;
	}

	delete[] m_pStudioHdr;
}

mstudioanim_t* CStudioModel::GetAnim( mstudioseqdesc_t* pseqdesc ) const
{
	mstudioseqgroup_t* pseqgroup = m_pStudioHdr->GetSequenceGroup( pseqdesc->seqgroup );

	if( pseqdesc->seqgroup == 0 )
	{
		return ( mstudioanim_t * ) ( ( byte * ) m_pStudioHdr + pseqgroup->unused2 + pseqdesc->animindex );
	}

	return ( mstudioanim_t * ) ( ( byte * ) m_pSeqHdrs[ pseqdesc->seqgroup ] + pseqdesc->animindex );
}

GLuint CStudioModel::GetTextureId( const int iIndex ) const
{
	const studiohdr_t* const pHdr = GetTextureHeader();

	if( !pHdr )
		return GL_INVALID_TEXTURE_ID;

	if( iIndex < 0 || iIndex >= pHdr->numtextures )
		return GL_INVALID_TEXTURE_ID;

	return m_Textures[ iIndex ];
}

namespace
{
/**
*	Loads a single studio header.
*/
StudioModelLoadResult LoadStudioHeader( const char* const pszFilename, const bool bAllowSeqGroup, studiohdr_t*& pOutStudioHdr )
{
	// load the model
	FILE* pFile = fopen( pszFilename, "rb" );

	if( !pFile )
		return StudioModelLoadResult::FAILURE;

	fseek( pFile, 0, SEEK_END );
	const size_t size = ftell( pFile );
	fseek( pFile, 0, SEEK_SET );

	std::unique_ptr<byte[]> buffer( new byte[ size ] );

	studiohdr_t* pStudioHdr = reinterpret_cast<studiohdr_t*>( buffer.get() );

	if( !pStudioHdr )
	{
		fclose( pFile );
		return StudioModelLoadResult::FAILURE;
	}

	const size_t uiRead = fread( pStudioHdr, size, 1, pFile );
	fclose( pFile );

	if( uiRead != 1 )
		return StudioModelLoadResult::FAILURE;

	if( strncmp( reinterpret_cast<const char*>( &pStudioHdr->id ), STUDIOMDL_HDR_ID, 4 ) &&
		strncmp( reinterpret_cast<const char*>( &pStudioHdr->id ), STUDIOMDL_SEQ_ID, 4 ) )
	{
		return StudioModelLoadResult::FAILURE;
	}

	if( !bAllowSeqGroup && !strncmp( reinterpret_cast<const char*>( &pStudioHdr->id ), STUDIOMDL_SEQ_ID, 4 ) )
	{
		return StudioModelLoadResult::FAILURE;
	}

	if( pStudioHdr->version != STUDIO_VERSION )
	{
		return StudioModelLoadResult::VERSIONDIFFERS;
	}

	pOutStudioHdr = pStudioHdr;

	buffer.release();

	return StudioModelLoadResult::SUCCESS;
}

void UploadRGBATexture( const int iWidth, const int iHeight, byte* pData, GLuint textureId, const bool bFilterTextures )
{
	glBindTexture( GL_TEXTURE_2D, textureId );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, iWidth, iHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, pData );
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, bFilterTextures ? GL_LINEAR : GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, bFilterTextures ? GL_LINEAR : GL_NEAREST );
}

void UploadTexture( const mstudiotexture_t* ptexture, const byte* data, const byte* pal, int name, const bool bFilterTextures )
{
	// unsigned *in, int inwidth, int inheight, unsigned *out,  int outwidth, int outheight;
	int		i, j;
	int		row1[ MAX_TEXTURE_DIMS ], row2[ MAX_TEXTURE_DIMS ], col1[ MAX_TEXTURE_DIMS ], col2[ MAX_TEXTURE_DIMS ];
	const byte	*pix1, *pix2, *pix3, *pix4;
	byte	*tex, *out;

	// convert texture to power of 2
	int outwidth;
	int outheight;

	if( !graphics::helpers::CalculateImageDimensions( ptexture->width, ptexture->height, outwidth, outheight ) )
		return;

	tex = out = ( byte * ) malloc( outwidth * outheight * 4 );
	if( !out )
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

	for( i = 0; i < outwidth; i++ )
	{
		col1[ i ] = ( int ) ( ( i + 0.25 ) * ( ptexture->width / ( float ) outwidth ) );
		col2[ i ] = ( int ) ( ( i + 0.75 ) * ( ptexture->width / ( float ) outwidth ) );
	}

	for( i = 0; i < outheight; i++ )
	{
		row1[ i ] = ( int ) ( ( i + 0.25 ) * ( ptexture->height / ( float ) outheight ) ) * ptexture->width;
		row2[ i ] = ( int ) ( ( i + 0.75 ) * ( ptexture->height / ( float ) outheight ) ) * ptexture->width;
	}

	const byte* const pAlpha = &pal[ PALETTE_ALPHA_INDEX ];

	// scale down and convert to 32bit RGB
	for( i = 0; i<outheight; i++ )
	{
		for( j = 0; j<outwidth; j++, out += 4 )
		{
			pix1 = &pal[ data[ row1[ i ] + col1[ j ] ] * 3 ];
			pix2 = &pal[ data[ row1[ i ] + col2[ j ] ] * 3 ];
			pix3 = &pal[ data[ row2[ i ] + col1[ j ] ] * 3 ];
			pix4 = &pal[ data[ row2[ i ] + col2[ j ] ] * 3 ];

			out[ 0 ] = ( pix1[ 0 ] + pix2[ 0 ] + pix3[ 0 ] + pix4[ 0 ] ) >> 2;
			out[ 1 ] = ( pix1[ 1 ] + pix2[ 1 ] + pix3[ 1 ] + pix4[ 1 ] ) >> 2;
			out[ 2 ] = ( pix1[ 2 ] + pix2[ 2 ] + pix3[ 2 ] + pix4[ 2 ] ) >> 2;

			if( ptexture->flags & STUDIO_NF_MASKED && pix1 == pAlpha && pix2 == pAlpha && pix3 == pAlpha && pix4 == pAlpha )
			{
				out[ 3 ] = 0x00;
			}
			else
			{
				out[ 3 ] = 0xFF;
			}
		}
	}

	UploadRGBATexture( outwidth, outheight, tex, name, bFilterTextures );

	free( tex );
}

size_t UploadTextures( const studiohdr_t& textureHdr, GLuint* pTextures, const bool bFilterTextures )
{
	size_t uiNumTextures = 0;

	if( textureHdr.textureindex > 0 && textureHdr.numtextures <= CStudioModel::MAX_TEXTURES )
	{
		const mstudiotexture_t* ptexture = textureHdr.GetTextures();

		const byte* pIn = reinterpret_cast<const byte*>( &textureHdr );

		const int n = textureHdr.numtextures;

		for( int i = 0; i < n; ++i )
		{
			GLuint name;

			glBindTexture( GL_TEXTURE_2D, 0 );
			glGenTextures( 1, &name );

			UploadTexture( &ptexture[ i ], pIn + ptexture[ i ].index, pIn + ptexture[ i ].width * ptexture[ i ].height + ptexture[ i ].index, name, bFilterTextures );

			pTextures[ i ] = name;
		}

		uiNumTextures = n;
	}

	return uiNumTextures;
}
}

StudioModelLoadResult LoadStudioModel( const char* const pszFilename, const bool bFilterTextures, CStudioModel*& pModel )
{
	//Takes care of cleanup on failure.
	std::unique_ptr<CStudioModel> studioModel( new CStudioModel() );

	//Load the model
	StudioModelLoadResult result = LoadStudioHeader( pszFilename, false, studioModel->m_pStudioHdr );

	if( result != StudioModelLoadResult::SUCCESS )
	{
		return result;
	}

	// preload textures
	if( studioModel->m_pStudioHdr->numtextures == 0 )
	{
		char texturename[ MAX_PATH_LENGTH ];

		strcpy( texturename, pszFilename );
		strcpy( &texturename[ strlen( texturename ) - 4 ], "T.mdl" );

		result = LoadStudioHeader( pszFilename, true, studioModel->m_pTextureHdr );

		if( result != StudioModelLoadResult::SUCCESS )
		{
			return result;
		}
	}
	else
	{
		studioModel->m_pTextureHdr = studioModel->m_pStudioHdr;
	}

	// preload animations
	if( studioModel->m_pStudioHdr->numseqgroups > 1 )
	{
		char seqgroupname[ MAX_PATH_LENGTH ];

		for( int i = 1; i < studioModel->m_pStudioHdr->numseqgroups; ++i )
		{
			strcpy( seqgroupname, pszFilename );
			snprintf( &seqgroupname[ strlen( seqgroupname ) - 4 ], sizeof( seqgroupname ), "%02d.mdl", i );

			result = LoadStudioHeader( seqgroupname, true, studioModel->m_pSeqHdrs[ i ] );

			if( result != StudioModelLoadResult::SUCCESS )
			{
				return result;
			}
		}
	}

	UploadTextures( *studioModel->m_pTextureHdr, studioModel->m_Textures, bFilterTextures );

	pModel = studioModel.release();

	return StudioModelLoadResult::SUCCESS;
}
}