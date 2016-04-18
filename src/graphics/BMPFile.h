#ifndef GRAPHICS_BMPFILE_H
#define GRAPHICS_BMPFILE_H

#include <cstdint>

namespace graphics
{
namespace bmpfile
{
//Required because the header is not aligned to > 2 bytes. TODO: figure out if other platforms need anything.
#ifdef WIN32
#include <pshpack2.h>
#endif

struct Header final
{
	uint16_t bfType;
	uint32_t bfSize;
	uint16_t bfReserved1;
	uint16_t bfReserved2;
	uint32_t bfOffBits;
};

struct InfoHeader final
{
	uint32_t biSize;
	int32_t biWidth;
	int32_t biHeight;
	uint16_t biPlanes;
	uint16_t biBitCount;
	uint32_t biCompression;
	uint32_t biSizeImage;
	int32_t biXPelsPerMeter;
	int32_t biYPelsPerMeter;
	uint32_t biClrUsed;
	uint32_t biClrImportant;
};

struct RGBQuad final
{
	uint8_t rgbBlue;
	uint8_t rgbGreen;
	uint8_t rgbRed;
	uint8_t rgbReserved;
};

#ifdef WIN32
#include <poppack.h>
#endif

#define BMP_TYPE_ID 0x4D42

enum Compression
{
	COMPRESSION_RGB = 0,
};

/**
*	Saves a BMP file. Note: does not support the full range of BMP's features. It is intended to be used only for use with HL textures.
*	@param pszFilename Filename to save to.
*	@param iWidth Width of the image.
*	@param iHeight Height of the image.
*	@param pPixels Array of pixels. Must be iWidth * iHeight bytes in size.
*	@param pPalette Array of colors. Must be 256 entries, each entry being 3 bytes (RGB 8 bit)
*	@return true on success, false otherwise.
*/
bool SaveBMPFile( const char* const pszFilename, const int iWidth, const int iHeight, const uint8_t* pPixels, const uint8_t* pPalette );
}
}

#endif //GRAPHICS_BMPFILE_H