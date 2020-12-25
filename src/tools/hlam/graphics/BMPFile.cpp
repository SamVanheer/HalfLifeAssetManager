#include <cstdio>
#include <cstring>
#include <memory>

#include "graphics/Palette.hpp"

#include "graphics/BMPFile.hpp"

#include "utility/IOUtils.hpp"

namespace graphics
{
namespace bmpfile
{
bool SaveBMPFile( const char* const pszFilename, const int iWidth, const int iHeight, const uint8_t* pPixels, const uint8_t* pPalette )
{
	if( !pszFilename || !( *pszFilename ) )
		return false;

	if( iWidth <= 0 || iHeight <= 0 )
		return false;

	if( !pPixels || !( pPalette ) )
		return false;

	FILE* pFile = utf8_fopen( pszFilename, "wb" );

	if( !pFile )
		return false;

	Header header;
	InfoHeader infoHeader;

	memset( &header, 0, sizeof( header ) );
	memset( &infoHeader, 0, sizeof( infoHeader ) );

	const int32_t iDiskWidth = ( iWidth + 3 ) & ~3;

	const int32_t iPixelsBytes = iDiskWidth * iHeight;

	const int32_t iPaletteBytes = PALETTE_ENTRIES * sizeof( RGBQuad );

	//This might not be correct. TODO: verify
	header.bfType		= BMP_TYPE_ID;

	header.bfSize		= sizeof( Header ) + sizeof( InfoHeader ) + iPixelsBytes + iPaletteBytes;
	header.bfReserved1	= 0;
	header.bfReserved2	= 0;
	header.bfOffBits	= sizeof( Header ) + sizeof( InfoHeader ) + iPaletteBytes;

	//Write the header.
	if( fwrite( &header, sizeof( header ), 1, pFile ) != 1 )
	{
		fclose( pFile );
		return false;
	}

	infoHeader.biSize			= sizeof( infoHeader );
	infoHeader.biWidth			= iDiskWidth;
	infoHeader.biHeight			= iHeight;
	infoHeader.biPlanes			= 1;
	infoHeader.biBitCount		= 8;						//8 bit encoding.
	infoHeader.biCompression	= COMPRESSION_RGB;			//Uncompressed.
	infoHeader.biSizeImage		= 0;
	infoHeader.biXPelsPerMeter	= 0;
	infoHeader.biYPelsPerMeter	= 0;
	infoHeader.biClrUsed		= PALETTE_ENTRIES;			//Store all colors.
	infoHeader.biClrImportant	= 0;

	//Write the info header.
	if( fwrite( &infoHeader, sizeof( infoHeader ), 1, pFile ) != 1 )
	{
		fclose( pFile );
		return false;
	}

	RGBQuad palette[ PALETTE_ENTRIES ];

	for( size_t uiIndex = 0; uiIndex < PALETTE_ENTRIES; ++uiIndex, pPalette += 3 )
	{
		palette[ uiIndex ].rgbRed		= pPalette[ 0 ];
		palette[ uiIndex ].rgbGreen		= pPalette[ 1 ];
		palette[ uiIndex ].rgbBlue		= pPalette[ 2 ];
		palette[ uiIndex ].rgbReserved	= 0;
	}

	//Write the palette
	if( fwrite( palette, sizeof( palette ), 1, pFile ) != 1 )
	{
		fclose( pFile );
		return false;
	}

	std::unique_ptr<uint8_t[]> pixels = std::make_unique<uint8_t[]>( iPixelsBytes );

	//Flip the image vertically.
	const uint8_t* pSrcData = pPixels + ( iHeight - 1 ) * iWidth;

	for( int iIndex = 0; iIndex < iHeight; ++iIndex, pSrcData -= iWidth )
	{
		memcpy( pixels.get() + ( iDiskWidth * iIndex ), pSrcData, iWidth );
	}

	if( fwrite( pixels.get(), iPixelsBytes, 1, pFile ) != 1 )
	{
		fclose( pFile );
		return false;
	}

	fclose( pFile );

	return true;
}
}
}