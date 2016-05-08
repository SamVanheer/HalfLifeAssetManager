#include <cassert>
#include <memory>

#include "shared/Platform.h"
#include "shared/Logging.h"

#include "cvar/CCVar.h"

#include "graphics/GraphicsHelpers.h"
#include "graphics/Palette.h"

#include "CStudioModel.h"

namespace studiomodel
{
namespace
{
static cvar::CCVar r_filtertextures( "r_filtertextures", cvar::CCVarArgsBuilder().FloatValue( 1 ).HelpInfo( "Whether to filter textures or not" ) );

void UploadRGBATexture( const int iWidth, const int iHeight, byte* pData, GLuint textureId, const bool bFilterTextures )
{
	glBindTexture( GL_TEXTURE_2D, textureId );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, iWidth, iHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, pData );
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, bFilterTextures ? GL_LINEAR : GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, bFilterTextures ? GL_LINEAR : GL_NEAREST );
}

void UploadTexture( const mstudiotexture_t* ptexture, const byte* data, byte* pal, int name, const bool bFilterTextures )
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

	const size_t uiSize = outwidth * outheight * 4;

	//Needs at least one pixel (satisfies code analysis)
	if( uiSize < 4 )
		return;

	tex = out = ( byte * ) malloc( uiSize );
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

	//This modifies the model's data. Sets the mask color to black. This is also done by Jed's model viewer. (export texture has black)
	if( ptexture->flags & STUDIO_NF_MASKED )
	{
		pal[ 255 * 3 + 0 ] = pal[ 255 * 3 + 1 ] = pal[ 255 * 3 + 2 ] = 0;
	}

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
				//Set alpha to 0 to enable transparent pixel.
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

size_t UploadTextures( studiohdr_t& textureHdr, GLuint* pTextures, const bool bFilterTextures )
{
	size_t uiNumTextures = 0;

	if( textureHdr.textureindex > 0 && textureHdr.numtextures <= CStudioModel::MAX_TEXTURES )
	{
		mstudiotexture_t* ptexture = textureHdr.GetTextures();

		byte* pIn = reinterpret_cast<byte*>( &textureHdr );

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

mstudiomodel_t* CStudioModel::GetModelByBodyPart( const int iBody, const int iBodyPart ) const
{
	mstudiobodyparts_t* pbodypart = m_pStudioHdr->GetBodypart( iBodyPart );

	int index = iBody / pbodypart->base;
	index = index % pbodypart->nummodels;

	return ( mstudiomodel_t * ) ( ( byte * ) m_pStudioHdr + pbodypart->modelindex ) + index;
}

bool CStudioModel::CalculateBodygroup( const int iGroup, const int iValue, int& iInOutBodygroup ) const
{
	if( iGroup > m_pStudioHdr->numbodyparts )
		return false;

	const mstudiobodyparts_t* const pbodypart = m_pStudioHdr->GetBodypart( iGroup );

	int iCurrent = ( iInOutBodygroup / pbodypart->base ) % pbodypart->nummodels;

	if( iValue >= pbodypart->nummodels )
		return true;

	iInOutBodygroup = ( iInOutBodygroup - ( iCurrent * pbodypart->base ) + ( iValue * pbodypart->base ) );

	return true;
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

void CStudioModel::ReplaceTexture( mstudiotexture_t* ptexture, byte *data, byte *pal, GLuint textureId )
{
	glDeleteTextures( 1, &textureId );

	UploadTexture( ptexture, data, pal, textureId, r_filtertextures.GetBool() );
}

void CStudioModel::ReuploadTexture( mstudiotexture_t* ptexture )
{
	assert( ptexture );

	const int iIndex = ptexture - m_pTextureHdr->GetTextures();

	if( iIndex < 0 || iIndex >= m_pTextureHdr->numtextures )
	{
		Error( "CStudioModel::ReuploadTexture: Invalid texture!" );
		return;
	}

	GLuint textureId = m_Textures[ iIndex ];

	glDeleteTextures( 1, &textureId );

	UploadTexture( ptexture, 
				   m_pTextureHdr->GetData() + ptexture->index, 
				   m_pTextureHdr->GetData() + ptexture->index + ptexture->width * ptexture->height, textureId, r_filtertextures.GetBool() );
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
}

StudioModelLoadResult LoadStudioModel( const char* const pszFilename, CStudioModel*& pModel )
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

		result = LoadStudioHeader( texturename, true, studioModel->m_pTextureHdr );

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

			if( !PrintfSuccess( snprintf( &seqgroupname[ strlen( seqgroupname ) - 4 ], sizeof( seqgroupname ), "%02d.mdl", i ), sizeof( seqgroupname ) ) )
				return StudioModelLoadResult::FAILURE;

			result = LoadStudioHeader( seqgroupname, true, studioModel->m_pSeqHdrs[ i ] );

			if( result != StudioModelLoadResult::SUCCESS )
			{
				return result;
			}
		}
	}

	UploadTextures( *studioModel->m_pTextureHdr, studioModel->m_Textures, r_filtertextures.GetBool() );

	pModel = studioModel.release();

	return StudioModelLoadResult::SUCCESS;
}

bool SaveStudioModel( const char* const pszFilename, const CStudioModel* const pModel )
{
	if( !pszFilename )
		return false;

	if( !pModel )
		return false;

	FILE* pFile = fopen( pszFilename, "wb" );

	if( !pFile )
		return false;

	const studiohdr_t* const pStudioHdr = pModel->GetStudioHeader();

	bool bSuccess = fwrite( pStudioHdr, sizeof( byte ), pStudioHdr->length, pFile ) == pStudioHdr->length;

	fclose( pFile );

	if( !bSuccess )
	{
		return false;
	}

	const studiohdr_t* const pTextureHdr = pModel->GetTextureHeader();

	// write texture model
	if( pTextureHdr != pStudioHdr )
	{
		char texturename[ MAX_PATH_LENGTH ];

		strcpy( texturename, pszFilename );
		strcpy( &texturename[ strlen( texturename ) - 4 ], "T.mdl" );

		pFile = fopen( texturename, "wb" );

		if( !pFile )
			return false;

		bSuccess = fwrite( pTextureHdr, sizeof( byte ), pTextureHdr->length, pFile ) == pTextureHdr->length;
		fclose( pFile );

		if( !bSuccess )
		{
			return false;
		}
	}

	// write seq groups
	if( pStudioHdr->numseqgroups > 1 )
	{
		char seqgroupname[ MAX_PATH_LENGTH ];

		for( int i = 1; i < pStudioHdr->numseqgroups; i++ )
		{
			strcpy( seqgroupname, pszFilename );
			sprintf( &seqgroupname[ strlen( seqgroupname ) - 4 ], "%02d.mdl", i );

			pFile = fopen( seqgroupname, "wb" );

			if( !pFile )
				return false;

			const auto pAnimHdr = pModel->GetSeqGroupHeader( i );

			bSuccess = fwrite( pAnimHdr, sizeof( byte ), pAnimHdr->length, pFile ) == pAnimHdr->length;
			fclose( pFile );

			if( !bSuccess )
			{
				return false;
			}
		}
	}

	return true;
}

void ScaleMeshes( CStudioModel* pStudioModel, const float flScale )
{
	assert( pStudioModel );

	auto pStudioHdr = pStudioModel->GetStudioHeader();

	int iBodygroup = 0;

	// scale verts
	for( int i = 0; i < pStudioHdr->numbodyparts; i++ )
	{
		mstudiobodyparts_t *pbodypart = pStudioHdr->GetBodypart( i );
		for( int j = 0; j < pbodypart->nummodels; j++ )
		{
			pStudioModel->CalculateBodygroup( i, j, iBodygroup );

			int bodypart = i;

			if( bodypart > pStudioHdr->numbodyparts )
			{
				// Con_DPrintf ("studiomodel::SetupModel: no such bodypart %d\n", bodypart);
				bodypart = 0;
			}

			mstudiomodel_t* pModel = pStudioModel->GetModelByBodyPart( iBodygroup, bodypart );

			glm::vec3 *pstudioverts = ( glm::vec3 * )( ( byte * ) pStudioHdr + pModel->vertindex );

			for( int k = 0; k < pModel->numverts; k++ )
			{
				pstudioverts[ k ] *= flScale;
			}
		}
	}

	// scale complex hitboxes
	mstudiobbox_t *pbboxes = pStudioHdr->GetHitBoxes();

	for( int i = 0; i < pStudioHdr->numhitboxes; i++ )
	{
		pbboxes[ i ].bbmin *= flScale;
		pbboxes[ i ].bbmax *= flScale;
	}

	// scale bounding boxes
	mstudioseqdesc_t *pseqdesc = pStudioHdr->GetSequences();

	for( int i = 0; i < pStudioHdr->numseq; i++ )
	{
		pseqdesc[ i ].bbmin *= flScale;
		pseqdesc[ i ].bbmax *= flScale;
	}

	// maybe scale exeposition, pivots, attachments
}

void ScaleBones( CStudioModel* pStudioModel, const float flScale )
{
	assert( pStudioModel );

	const auto pStudioHdr = pStudioModel->GetStudioHeader();

	mstudiobone_t* const pbones = pStudioHdr->GetBones();

	for( int i = 0; i < pStudioHdr->numbones; i++ )
	{
		for( int j = 0; j < 3; j++ )
		{
			pbones[ i ].value[ j ] *= flScale;
			pbones[ i ].scale[ j ] *= flScale;
		}
	}
}

const char* ControlToString( const int iControl )
{
	switch( iControl )
	{
	case STUDIO_X:		return "X";
	case STUDIO_Y:		return "Y";
	case STUDIO_Z:		return "Z";
	case STUDIO_XR:		return "XR";
	case STUDIO_YR:		return "YR";
	case STUDIO_ZR:		return "ZR";
	case STUDIO_LX:		return "LX";
	case STUDIO_LY:		return "LY";
	case STUDIO_LZ:		return "LZ";
	case STUDIO_AX:		return "AX";
	case STUDIO_AY:		return "AY";
	case STUDIO_AZ:		return "AZ";
	case STUDIO_AXR:	return "AXR";
	case STUDIO_AYR:	return "AYR";
	case STUDIO_AZR:	return "AZR";

	default:			return nullptr;
	}
}

const char* ControlToStringDescription( const int iControl )
{
	switch( iControl )
	{
	case STUDIO_X:		return "Sequence motiontype flag: Zeroes out movement in the X axis\nBone controller type flag: Sets X offset";
	case STUDIO_Y:		return "Sequence motiontype flag: Zeroes out movement in the Y axis\nBone controller type flag: Sets Y offset";
	case STUDIO_Z:		return "Sequence motiontype flag: Zeroes out movement in the Z axis\nBone controller type flag: Sets Z offset";
	case STUDIO_XR:		return "Bone controller type flag: Sets X rotation\nSequence blendtype: Controls blending range clamping in the X axis";
	case STUDIO_YR:		return "Bone controller type flag: Sets Y rotation\nSequence blendtype: Controls blending range clamping in the Y axis";
	case STUDIO_ZR:		return "Bone controller type flag: Sets Z rotation\nSequence blendtype: Controls blending range clamping in the Z axis";
	case STUDIO_LX:		return "StudioMdl Compiler flag: Sets the sequence's last frame's X axis position to the first frame's position";
	case STUDIO_LY:		return "StudioMdl Compiler flag: Sets the sequence's last frame's Y axis position to the first frame's position";
	case STUDIO_LZ:		return "StudioMdl Compiler flag: Sets the sequence's last frame's Z axis position to the first frame's position";
	case STUDIO_AX:		return "StudioMdl Compiler flag: Extract relative movement distance between the first and current frame in the X axis (Unused)";
	case STUDIO_AY:		return "StudioMdl Compiler flag: Extract relative movement distance between the first and current frame in the Y axis (Unused)";
	case STUDIO_AZ:		return "StudioMdl Compiler flag: Extract relative movement distance between the first and current frame in the Z axis (Unused)";
	case STUDIO_AXR:	return "StudioMdl Compiler flag: Extract relative rotation distance between the first and current frame around the X axis (Unused)";
	case STUDIO_AYR:	return "StudioMdl Compiler flag: Extract relative rotation distance between the first and current frame around the Y axis (Unused)";
	case STUDIO_AZR:	return "StudioMdl Compiler flag: Extract relative rotation distance between the first and current frame around the Z axis (Unused)";

	default:			return nullptr;
	}
}

int StringToControl( const char* const pszString )
{
	assert( pszString );

	if( strcasecmp( pszString, "X" ) == 0 ) return STUDIO_X;
	if( strcasecmp( pszString, "Y" ) == 0 ) return STUDIO_Y;
	if( strcasecmp( pszString, "Z" ) == 0 ) return STUDIO_Z;
	if( strcasecmp( pszString, "XR" ) == 0 ) return STUDIO_XR;
	if( strcasecmp( pszString, "YR" ) == 0 ) return STUDIO_YR;
	if( strcasecmp( pszString, "ZR" ) == 0 ) return STUDIO_ZR;
	if( strcasecmp( pszString, "LX" ) == 0 ) return STUDIO_LX;
	if( strcasecmp( pszString, "LY" ) == 0 ) return STUDIO_LY;
	if( strcasecmp( pszString, "LZ" ) == 0 ) return STUDIO_LZ;
	if( strcasecmp( pszString, "AX" ) == 0 ) return STUDIO_AX;
	if( strcasecmp( pszString, "AY" ) == 0 ) return STUDIO_AY;
	if( strcasecmp( pszString, "AZ" ) == 0 ) return STUDIO_AZ;
	if( strcasecmp( pszString, "AXR" ) == 0 ) return STUDIO_AXR;
	if( strcasecmp( pszString, "AYR" ) == 0 ) return STUDIO_AYR;
	if( strcasecmp( pszString, "AZR" ) == 0 ) return STUDIO_AZR;

	return -1;
}
}