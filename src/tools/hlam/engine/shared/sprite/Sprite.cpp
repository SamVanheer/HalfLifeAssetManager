#include <cassert>
#include <cstdio>
#include <memory>

#include "core/shared/Const.hpp"

#include "utility/ByteSwap.hpp"
#include "utility/IOUtils.hpp"

#include "graphics/Palette.hpp"

#include "engine/shared/sprite/Sprite.hpp"

namespace sprite
{
namespace
{
/**
*	Converts an 8 bit indexed palette into a 32 bit palette using the given format.
*	@param pInPalette 8 bit indexed palette.
*	@param pRGBAPalette 32 bit RGBA palette.
*	@param format Texture format to convert to.
*/
void Convert8To32Bit( const byte* pInPalette, byte* pRGBAPalette, const TexFormat::TexFormat format )
{
	assert( pInPalette );
	assert( pRGBAPalette );

	byte* pRGBA = pRGBAPalette;

	switch( format )
	{
	default: //TODO: warn
	case TexFormat::SPR_NORMAL:
	case TexFormat::SPR_ADDITIVE:
		{
			for( size_t uiIndex = 0; uiIndex < PALETTE_ENTRIES; ++uiIndex, pRGBA += 4 )
			{
				pRGBA[ 0 ] = pInPalette[ uiIndex * 3 ];
				pRGBA[ 1 ] = pInPalette[ uiIndex * 3 + 1 ];
				pRGBA[ 2 ] = pInPalette[ uiIndex * 3 + 2 ];
				pRGBA[ 3 ] = 0xFF;
			}

			break;
		}

	case TexFormat::SPR_INDEXALPHA:
		{
			for( size_t uiIndex = 0; uiIndex < PALETTE_ENTRIES; ++uiIndex, pRGBA += 4 )
			{
				pRGBA[ 0 ] = pInPalette[ uiIndex * 3 ];
				pRGBA[ 1 ] = pInPalette[ uiIndex * 3 + 1 ];
				pRGBA[ 2 ] = pInPalette[ uiIndex * 3 + 2 ];
				pRGBA[ 3 ] = static_cast<byte>( uiIndex );
			}

			break;
		}

	case TexFormat::SPR_ALPHTEST:
		{
			for( size_t uiIndex = 0; uiIndex < PALETTE_ENTRIES; ++uiIndex, pRGBA += 4 )
			{
				pRGBA[ 0 ] = pInPalette[ uiIndex * 3 ];
				pRGBA[ 1 ] = pInPalette[ uiIndex * 3 + 1 ];
				pRGBA[ 2 ] = pInPalette[ uiIndex * 3 + 2 ];
				pRGBA[ 3 ] = pInPalette[ uiIndex ] == 255 ? pInPalette[ uiIndex ] : 0xFF;
			}

			//Zero out the transparent color.
			memset( pRGBAPalette + 255 * 4, 0, sizeof( byte ) * 4 );

			break;
		}
	}
}

byte* LoadSpriteFrame( byte* pIn, mspriteframe_t** ppFrame, const int iFrame, const byte* pRGBAPalette )
{
	assert( pIn );
	assert( ppFrame );

	dspriteframe_t* pFrame = reinterpret_cast<dspriteframe_t*>( pIn );

	const int iWidth = LittleValue( pFrame->width );
	const int iHeight = LittleValue( pFrame->height );

	const glm::ivec2 vecOrigin{ LittleValue( pFrame->origin[ 0 ] ), LittleValue( pFrame->origin[ 1 ] ) };

	mspriteframe_t* pSpriteFrame = new mspriteframe_t;

	memset( pSpriteFrame, 0, sizeof( mspriteframe_t ) );

	*ppFrame = pSpriteFrame;

	pSpriteFrame->width		= LittleValue( pFrame->width );
	pSpriteFrame->height	= LittleValue( pFrame->height );

	pSpriteFrame->up		= static_cast<float>( vecOrigin[ 1 ] );
	pSpriteFrame->down		= static_cast<float>( vecOrigin[ 1 ] - iHeight );
	pSpriteFrame->left		= static_cast<float>( vecOrigin[ 0 ] );
	pSpriteFrame->right		= static_cast<float>( iWidth + vecOrigin[ 0 ] );

	byte* pPixelData = reinterpret_cast<byte*>( pFrame + 1 );

	glGenTextures( 1, &pSpriteFrame->gl_texturenum );

	std::unique_ptr<byte[]> rgba = std::make_unique<byte[]>( iWidth * iHeight * 4 );

	byte* out  = rgba.get();

	for( int i = 0; i<iWidth; i++ )
	{
		for( int j = 0; j<iHeight; j++, out += 4 )
		{
			out[ 0 ] = pRGBAPalette[ pPixelData[ i * pSpriteFrame->width + j ] * 4 ];
			out[ 1 ] = pRGBAPalette[ pPixelData[ i * pSpriteFrame->width + j ] * 4 + 1 ];
			out[ 2 ] = pRGBAPalette[ pPixelData[ i * pSpriteFrame->width + j ] * 4 + 2 ];
			out[ 3 ] = pRGBAPalette[ pPixelData[ i * pSpriteFrame->width + j ] * 4 + 3 ];
		}
	}

	//TODO: this is the same code as used by studiomodel. Refactor.
	//TODO: it might be better to upload sprites as a single large texture containing all frames.
	glBindTexture( GL_TEXTURE_2D, pSpriteFrame->gl_texturenum );
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, iWidth, iHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba.get() );
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

	return pPixelData + ( iWidth * iHeight );
}

byte* LoadSpriteGroup( byte* pIn, mspriteframe_t** ppFrame, const int iFrame, const byte* pRGBAPalette )
{
	dspritegroup_t* pGroup = reinterpret_cast<dspritegroup_t*>( pIn );

	const int iNumFrames = LittleValue( pGroup->numframes );

	const size_t size = sizeof( mspritegroup_t ) + ( ( iNumFrames - 1 ) * sizeof( mspriteframe_t* ) );

	mspritegroup_t* pSpriteGroup = reinterpret_cast<mspritegroup_t*>( new byte[ size ] );

	memset( pSpriteGroup, 0, size );

	*ppFrame = reinterpret_cast<mspriteframe_t*>( pSpriteGroup );

	pSpriteGroup->numframes = iNumFrames;

	float* pInIntervals = reinterpret_cast<float*>( pGroup + 1 );

	float* pOutIntervals = pSpriteGroup->intervals = new float[ iNumFrames ];

	for( int iIndex = 0; iIndex < iNumFrames; ++iIndex, ++pInIntervals, ++pOutIntervals )
	{
		*pOutIntervals = LittleValue( *pInIntervals );

		//TODO: error checking
	}

	byte* pInput = reinterpret_cast<byte*>( pInIntervals );

	for( int iIndex = 0; iIndex < iNumFrames; ++iIndex )
	{
		pInput = LoadSpriteFrame( pInput, &pSpriteGroup->frames[ iIndex ], iFrame * 100 + iIndex, pRGBAPalette );
	}

	return pInput;
}

bool LoadSpriteInternal( byte* pIn, msprite_t*& pSprite )
{
	assert( pIn );

	dsprite_t* pHeader = reinterpret_cast<dsprite_t*>( pIn );

	if( LittleValue( pHeader->version ) != SPRITE_VERSION )
		return false;

	if( LittleValue( pHeader->ident ) != SPRITE_ID )
		return false;

	//Offset in the buffer where the frames are located
	size_t uiFrameOffset = 0;

	byte* pPalette = nullptr;

	if( *reinterpret_cast<short*>( pHeader + 1 ) == PALETTE_ENTRIES )
	{
		pPalette = reinterpret_cast<byte*>( reinterpret_cast<short*>( pHeader + 1 ) + 1 );

		uiFrameOffset = ( pPalette + PALETTE_SIZE ) - pIn;
	}
	else
	{
		return false;
	}

	const TexFormat::TexFormat texFormat = LittleEnumValue( pHeader->texFormat );

	byte convertedPalette[ PALETTE_ENTRIES * 4 ];

	Convert8To32Bit( pPalette, convertedPalette, texFormat );

	const int iNumFrames = LittleValue( pHeader->numframes );

	const size_t size = sizeof( msprite_t ) + ( sizeof( mspriteframedesc_t ) * iNumFrames - 1 );

	pSprite = reinterpret_cast<msprite_t*>( new byte[ size ] );

	memset( pSprite, 0, size );

	pSprite->type		= LittleEnumValue( pHeader->type );
	pSprite->texFormat	= texFormat;
	pSprite->maxwidth	= LittleValue( pHeader->width );
	pSprite->maxheight	= LittleValue( pHeader->height );
	pSprite->numframes	= iNumFrames;
	pSprite->beamlength	= LittleValue( pHeader->beamlength );
	//TODO: sync type

	//Load frames

	spriteframetype_t* pType = reinterpret_cast<spriteframetype_t*>( pIn + uiFrameOffset );

	for( int iFrame = 0; iFrame < iNumFrames; ++iFrame )
	{
		const spriteframetype_t type = LittleEnumValue( *pType );
		
		pSprite->frames[ iFrame ].type = type;

		if( type == spriteframetype_t::SINGLE )
		{
			pType = reinterpret_cast<spriteframetype_t*>( LoadSpriteFrame( reinterpret_cast<byte*>( pType + 1 ), &pSprite->frames[ iFrame ].frameptr, iFrame, convertedPalette ) );
		}
		else
		{
			pType = reinterpret_cast<spriteframetype_t*>( LoadSpriteGroup( reinterpret_cast<byte*>( pType + 1 ), &pSprite->frames[ iFrame ].frameptr, iFrame, convertedPalette ) );
		}
	}

	return true;
}
}

bool LoadSprite( const char* const pszFilename, msprite_t*& pSprite )
{
	assert( pszFilename );

	pSprite = nullptr;

	FILE* pFile = utf8_fopen( pszFilename, "rb" );

	if( !pFile )
		return false;

	const auto pos = ftell( pFile );
	fseek( pFile, 0, SEEK_END );
	const auto size = ftell( pFile );
	fseek( pFile, pos, SEEK_SET );

	std::unique_ptr<byte[]> pBuffer = std::make_unique<byte[]>( size );

	bool bSuccess = fread( pBuffer.get(), size, 1, pFile ) == 1;

	fclose( pFile );

	if( bSuccess )
	{
		bSuccess = LoadSpriteInternal( pBuffer.get(), pSprite );
	}

	if( !bSuccess )
	{
		FreeSprite( pSprite );
		pSprite = nullptr;
	}

	return bSuccess;
}

void FreeSprite( msprite_t* pSprite )
{
	if( !pSprite )
		return;

	for( int iFrame = 0; iFrame < pSprite->numframes; ++iFrame )
	{
		if( pSprite->frames[ iFrame ].type == spriteframetype_t::SINGLE )
		{
			delete pSprite->frames[ iFrame ].frameptr;
		}
		else
		{
			mspritegroup_t* pGroup = reinterpret_cast<mspritegroup_t*>( pSprite->frames[ iFrame ].frameptr );

			if( pGroup )
			{
				delete[] pGroup->intervals;

				for( int iGroupFrame = 0; iGroupFrame < pGroup->numframes; ++iGroupFrame )
				{
					delete pGroup->frames[ iGroupFrame ];
				}

				delete pGroup;
			}
		}
	}

	delete[] pSprite;
}
}