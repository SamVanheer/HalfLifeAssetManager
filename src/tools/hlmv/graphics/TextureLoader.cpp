#include <vector>

#include "graphics/Palette.hpp"
#include "graphics/TextureLoader.hpp"

namespace graphics
{
TextureLoader::TextureLoader() = default;
TextureLoader::~TextureLoader() = default;

void TextureLoader::UploadRGBA8888(GLuint texture, int width, int height, const byte* rgbaPixels, bool masked)
{
	const auto [newWidth, newHeight] = AdjustImageDimensions(width, height);

	std::vector<byte> pixels;

	if (newWidth != width || newHeight != height)
	{
		std::vector<int> col1, col2;
		std::vector<int> row1, row2;
		
		col1.resize(newWidth);
		col2.resize(newWidth);

		row1.resize(newHeight);
		row2.resize(newHeight);

		for (int i = 0; i < newWidth; ++i)
		{
			col1[i] = (int)((i + 0.25) * (width / (float)newWidth));
			col2[i] = (int)((i + 0.75) * (width / (float)newWidth));
		}

		for (int i = 0; i < newHeight; ++i)
		{
			row1[i] = (int)((i + 0.25) * (height / (float)newHeight)) * width;
			row2[i] = (int)((i + 0.75) * (height / (float)newHeight)) * width;
		}

		pixels.resize(newWidth * newHeight * 4);

		for (int i = 0; i < newHeight; ++i)
		{
			for (int j = 0; j < newWidth; ++j)
			{
				const auto pix1 = &rgbaPixels[(row1[i] + col1[j]) * 4];
				const auto pix2 = &rgbaPixels[(row1[i] + col2[j]) * 4];
				const auto pix3 = &rgbaPixels[(row2[i] + col1[j]) * 4];
				const auto pix4 = &rgbaPixels[(row2[i] + col2[j]) * 4];

				byte* const pixel = &pixels[((newWidth * i) + j) * 4];

				for (int p = 0; p < 4; ++p)
				{
					pixel[p] = (pix1[p] + pix2[p] + pix3[p] + pix4[p]) / 4;
				}

				//If any of the sampled pixels are transparent the destination pixel is also transparent
				if (masked && pixel[3] != 0xFF)
				{
					pixel[3] = 0x00;
				}
			}
		}

		rgbaPixels = pixels.data();
	}

	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, newWidth, newHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgbaPixels);
	SetFilters(texture);
}

void TextureLoader::UploadIndexed8(GLuint texture, int width, int height, const byte* pixels, const byte* palette, bool masked)
{
	//TODO: total size can be too large
	byte localPalette[PALETTE_SIZE];

	memcpy(localPalette, palette, sizeof(localPalette));

	//Sets the mask color to black. This helps limit the bleedover effect caused by resizing and filtering
	if (masked)
	{
		localPalette[255 * 3 + 0] = localPalette[255 * 3 + 1] = localPalette[255 * 3 + 2] = 0;
	}

	std::vector<byte> rgbaPixels;

	rgbaPixels.resize(width * height * 4);

	for (int i = 0; i < (width * height); ++i)
	{
		for (int c = 0; c < 3; ++c)
		{
			rgbaPixels[(i * 4) + c] = localPalette[(pixels[i] * 3) + c];
		}

		//For masked textures the last color in the table is the transparent color
		//Pixels with that color have their alpha value set to 0 to appear transparent
		if (masked && pixels[i] == 255)
		{
			rgbaPixels[(i * 4) + 3] = 0x00;
		}
		else
		{
			rgbaPixels[(i * 4) + 3] = 0xFF;
		}
	}

	UploadRGBA8888(texture, width, height, rgbaPixels.data(), masked);
}

void TextureLoader::SetFilters(GLuint texture)
{
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, ShouldFilterTextures() ? GL_LINEAR : GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, ShouldFilterTextures() ? GL_LINEAR : GL_NEAREST);
}

std::pair<int, int> TextureLoader::AdjustImageDimensions(int width, int height) const
{
	if (!ShouldResizeToPowerOf2())
	{
		return {width, height};
	}

	int newWidth, newHeight;

	for (newWidth = 1; newWidth > 0 && newWidth < width; newWidth <<= 1)
	{
	}

	for (newHeight = 1; newHeight > 0 && newHeight < height; newHeight <<= 1)
	{
	}

	//If the initial dimensions exceed the largest power of 2 then it should be clamped to that value
	if (newWidth < 0)
	{
		newWidth = 1 << 31;
	}

	if (newHeight < 0)
	{
		newHeight = 1 << 31;
	}

	return {newWidth, newHeight};
}
}
