#include <cassert>
#include <cstddef>
#include <cstdio>
#include <memory>

#include "formats/sprite/Sprite.hpp"

#include "graphics/Palette.hpp"

#include "utility/IOUtils.hpp"

namespace sprite
{
namespace
{
/**
*	Converts an 8 bit indexed palette into a 32 bit palette using the given format.
*	@param inPalette 8 bit indexed palette.
*	@param rgbaPalette 32 bit RGBA palette.
*	@param format Texture format to convert to.
*/
void Convert8To32Bit(const graphics::RGBPalette& inPalette, graphics::RGBAPalette& rgbaPalette, const TexFormat::TexFormat format)
{
	switch (format)
	{
	default: //TODO: warn
	case TexFormat::SPR_NORMAL:
	case TexFormat::SPR_ADDITIVE:
	{
		for (std::size_t i = 0; i < inPalette.size(); ++i)
		{
			rgbaPalette[i] = inPalette[i];
			rgbaPalette[i].A = 0xFF;
		}
		break;
	}

	case TexFormat::SPR_INDEXALPHA:
	{
		for (std::size_t i = 0; i < inPalette.size(); ++i)
		{
			rgbaPalette[i] = inPalette[i];
			rgbaPalette[i].A = static_cast<std::uint8_t>(i);
		}
		break;
	}

	case TexFormat::SPR_ALPHTEST:
	{
		for (std::size_t i = 0; i < inPalette.size(); ++i)
		{
			rgbaPalette[i] = inPalette[i];
			rgbaPalette[i].A = 0xFF;
		}

		//Zero out the transparent color.
		rgbaPalette.GetAlpha() = {0, 0, 0, 0};
		break;
	}
	}
}

std::byte* LoadSpriteFrame(QOpenGLFunctions_1_1* openglFunctions, std::byte* pIn, mspriteframe_t** ppFrame, const int iFrame, const graphics::RGBAPalette& rgbaPalette)
{
	assert(pIn);
	assert(ppFrame);

	dspriteframe_t* pFrame = reinterpret_cast<dspriteframe_t*>(pIn);

	const int iWidth = pFrame->width;
	const int iHeight = pFrame->height;

	const glm::ivec2 vecOrigin{pFrame->origin[0], pFrame->origin[1]};

	mspriteframe_t* pSpriteFrame = new mspriteframe_t;

	memset(pSpriteFrame, 0, sizeof(mspriteframe_t));

	*ppFrame = pSpriteFrame;

	pSpriteFrame->width = pFrame->width;
	pSpriteFrame->height = pFrame->height;

	pSpriteFrame->up = static_cast<float>(vecOrigin[1]);
	pSpriteFrame->down = static_cast<float>(vecOrigin[1] - iHeight);
	pSpriteFrame->left = static_cast<float>(vecOrigin[0]);
	pSpriteFrame->right = static_cast<float>(iWidth + vecOrigin[0]);

	auto pPixelData = reinterpret_cast<std::byte*>(pFrame + 1);

	openglFunctions->glGenTextures(1, &pSpriteFrame->gl_texturenum);

	auto rgba = std::make_unique<std::byte[]>(iWidth * iHeight * 4);

	std::byte* out = rgba.get();

	for (int i = 0; i < iWidth; i++)
	{
		for (int j = 0; j < iHeight; j++, out += 4)
		{
			out[0] = std::byte{rgbaPalette[std::to_integer<int>(pPixelData[i * pSpriteFrame->width + j])].R};
			out[1] = std::byte{rgbaPalette[std::to_integer<int>(pPixelData[i * pSpriteFrame->width + j])].G};
			out[2] = std::byte{rgbaPalette[std::to_integer<int>(pPixelData[i * pSpriteFrame->width + j])].B};
			out[3] = std::byte{rgbaPalette[std::to_integer<int>(pPixelData[i * pSpriteFrame->width + j])].A};
		}
	}

	//TODO: this is the same code as used by studiomodel. Refactor.
	//TODO: it might be better to upload sprites as a single large texture containing all frames.
	openglFunctions->glBindTexture(GL_TEXTURE_2D, pSpriteFrame->gl_texturenum);
	openglFunctions->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, iWidth, iHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba.get());
	openglFunctions->glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	openglFunctions->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	openglFunctions->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	return pPixelData + (iWidth * iHeight);
}

std::byte* LoadSpriteGroup(QOpenGLFunctions_1_1* openglFunctions, std::byte* pIn, mspriteframe_t** ppFrame, const int iFrame, const graphics::RGBAPalette& rgbaPalette)
{
	dspritegroup_t* pGroup = reinterpret_cast<dspritegroup_t*>(pIn);

	const int iNumFrames = pGroup->numframes;

	const size_t size = sizeof(mspritegroup_t) + ((iNumFrames - 1) * sizeof(mspriteframe_t*));

	mspritegroup_t* pSpriteGroup = reinterpret_cast<mspritegroup_t*>(new std::byte[size]);

	memset(pSpriteGroup, 0, size);

	*ppFrame = reinterpret_cast<mspriteframe_t*>(pSpriteGroup);

	pSpriteGroup->numframes = iNumFrames;

	float* pInIntervals = reinterpret_cast<float*>(pGroup + 1);

	float* pOutIntervals = pSpriteGroup->intervals = new float[iNumFrames];

	for (int iIndex = 0; iIndex < iNumFrames; ++iIndex, ++pInIntervals, ++pOutIntervals)
	{
		*pOutIntervals = *pInIntervals;

		//TODO: error checking
	}

	auto pInput = reinterpret_cast<std::byte*>(pInIntervals);

	for (int iIndex = 0; iIndex < iNumFrames; ++iIndex)
	{
		pInput = LoadSpriteFrame(openglFunctions, pInput, &pSpriteGroup->frames[iIndex], iFrame * 100 + iIndex, rgbaPalette);
	}

	return pInput;
}

bool LoadSpriteInternal(QOpenGLFunctions_1_1* openglFunctions, std::byte* pIn, msprite_t*& pSprite)
{
	assert(pIn);

	dsprite_t* pHeader = reinterpret_cast<dsprite_t*>(pIn);

	if (pHeader->version != SPRITE_VERSION)
		return false;

	if (pHeader->ident != SPRITE_ID)
		return false;

	if (*reinterpret_cast<short*>(pHeader + 1) != graphics::RGBPalette::EntriesCount)
	{
		return false;
	}

	const auto& palette = *reinterpret_cast<const graphics::RGBPalette*>(reinterpret_cast<short*>(pHeader + 1) + 1);

	//Offset in the buffer where the frames are located
	const size_t uiFrameOffset = reinterpret_cast<const std::byte*>(&palette) + palette.GetSizeInBytes() - pIn;

	const TexFormat::TexFormat texFormat = pHeader->texFormat;

	graphics::RGBAPalette convertedPalette;

	Convert8To32Bit(palette, convertedPalette, texFormat);

	const int iNumFrames = pHeader->numframes;

	const size_t size = sizeof(msprite_t) + (sizeof(mspriteframedesc_t) * iNumFrames - 1);

	pSprite = reinterpret_cast<msprite_t*>(new std::byte[size]);

	memset(pSprite, 0, size);

	pSprite->type = pHeader->type;
	pSprite->texFormat = texFormat;
	pSprite->maxwidth = pHeader->width;
	pSprite->maxheight = pHeader->height;
	pSprite->numframes = iNumFrames;
	pSprite->beamlength = pHeader->beamlength;
	//TODO: sync type

	//Load frames

	spriteframetype_t* pType = reinterpret_cast<spriteframetype_t*>(pIn + uiFrameOffset);

	for (int iFrame = 0; iFrame < iNumFrames; ++iFrame)
	{
		const spriteframetype_t type = *pType;

		pSprite->frames[iFrame].type = type;

		if (type == spriteframetype_t::SINGLE)
		{
			pType = reinterpret_cast<spriteframetype_t*>(LoadSpriteFrame(openglFunctions, reinterpret_cast<std::byte*>(pType + 1), &pSprite->frames[iFrame].frameptr, iFrame, convertedPalette));
		}
		else
		{
			pType = reinterpret_cast<spriteframetype_t*>(LoadSpriteGroup(openglFunctions, reinterpret_cast<std::byte*>(pType + 1), &pSprite->frames[iFrame].frameptr, iFrame, convertedPalette));
		}
	}

	return true;
}
}

bool LoadSprite(QOpenGLFunctions_1_1* openglFunctions, const char* const pszFilename, msprite_t*& pSprite)
{
	assert(pszFilename);

	pSprite = nullptr;

	FILE* pFile = utf8_fopen(pszFilename, "rb");

	if (!pFile)
		return false;

	const auto pos = ftell(pFile);
	fseek(pFile, 0, SEEK_END);
	const auto size = ftell(pFile);
	fseek(pFile, pos, SEEK_SET);

	auto pBuffer = std::make_unique<std::byte[]>(size);

	bool bSuccess = fread(pBuffer.get(), size, 1, pFile) == 1;

	fclose(pFile);

	if (bSuccess)
	{
		bSuccess = LoadSpriteInternal(openglFunctions, pBuffer.get(), pSprite);
	}

	if (!bSuccess)
	{
		FreeSprite(openglFunctions, pSprite);
		pSprite = nullptr;
	}

	return bSuccess;
}

void FreeSprite(QOpenGLFunctions_1_1* openglFunctions, msprite_t* pSprite)
{
	if (!pSprite)
		return;

	// TODO: free opengl resources

	for (int iFrame = 0; iFrame < pSprite->numframes; ++iFrame)
	{
		if (pSprite->frames[iFrame].type == spriteframetype_t::SINGLE)
		{
			delete pSprite->frames[iFrame].frameptr;
		}
		else
		{
			mspritegroup_t* pGroup = reinterpret_cast<mspritegroup_t*>(pSprite->frames[iFrame].frameptr);

			if (pGroup)
			{
				delete[] pGroup->intervals;

				for (int iGroupFrame = 0; iGroupFrame < pGroup->numframes; ++iGroupFrame)
				{
					delete pGroup->frames[iGroupFrame];
				}

				delete pGroup;
			}
		}
	}

	delete[] pSprite;
}
}