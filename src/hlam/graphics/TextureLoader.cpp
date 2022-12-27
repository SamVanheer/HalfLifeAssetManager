#include <algorithm>
#include <cstdint>
#include <cstring>
#include <vector>

#include <QOpenGLFunctions_1_1>

#include "graphics/Palette.hpp"
#include "graphics/TextureLoader.hpp"

namespace graphics
{
TextureLoader::TextureLoader(QOpenGLFunctions_1_1* openglFunctions)
	: _openglFunctions(openglFunctions)
{
	SetTextureFilters(TextureFilter::Linear, TextureFilter::Linear, MipmapFilter::None);
}

TextureLoader::~TextureLoader() = default;

void TextureLoader::SetTextureFilters(TextureFilter minFilter, TextureFilter magFilter, MipmapFilter mipmapFilter)
{
	_minFilter = minFilter;
	_magFilter = magFilter;
	_mipmapFilter = mipmapFilter;

	switch (_minFilter)
	{
	case TextureFilter::Point:
	{
		switch (_mipmapFilter)
		{
		case MipmapFilter::None:
			_glMinFilter = GL_NEAREST;
			break;

		case MipmapFilter::Point:
			_glMinFilter = GL_NEAREST_MIPMAP_NEAREST;
			break;

		case MipmapFilter::Linear:
			_glMinFilter = GL_NEAREST_MIPMAP_LINEAR;
		}
		break;
	}

	case TextureFilter::Linear:
	{
		switch (_mipmapFilter)
		{
		case MipmapFilter::None:
			_glMinFilter = GL_LINEAR;
			break;

		case MipmapFilter::Point:
			_glMinFilter = GL_LINEAR_MIPMAP_NEAREST;
			break;

		case MipmapFilter::Linear:
			_glMinFilter = GL_LINEAR_MIPMAP_LINEAR;
		}
		break;
	}
	}

	switch (_magFilter)
	{
	case TextureFilter::Point:
	{
		_glMagFilter = GL_NEAREST;
		break;
	}

	case TextureFilter::Linear:
	{
		_glMagFilter = GL_LINEAR;
		break;
	}
	}
}

GLuint TextureLoader::CreateTexture()
{
	GLuint texture = 0;
	_openglFunctions->glGenTextures(1, &texture);
	return texture;
}

void TextureLoader::DeleteTexture(GLuint texture)
{
	_openglFunctions->glDeleteTextures(1, &texture);
}

void TextureLoader::UploadRGBA8888(GLuint texture, int width, int height, const std::byte* rgbaPixels, bool generateMipmaps, bool masked)
{
	const auto [newWidth, newHeight] = AdjustImageDimensions(width, height);

	std::vector<std::byte> pixels;

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

				std::byte* const pixel = &pixels[((newWidth * i) + j) * 4];

				for (int p = 0; p < 4; ++p)
				{
					pixel[p] = std::byte((std::to_integer<int>(pix1[p])
						+ std::to_integer<int>(pix2[p])
						+ std::to_integer<int>(pix3[p])
						+ std::to_integer<int>(pix4[p])) / 4);
				}

				//If any of the sampled pixels are transparent the destination pixel is also transparent
				if (masked && pixel[3] != std::byte{0xFF})
				{
					pixel[3] = std::byte{0x00};
				}
			}
		}

		rgbaPixels = pixels.data();
	}

	_openglFunctions->glBindTexture(GL_TEXTURE_2D, texture);
	_openglFunctions->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, newWidth, newHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgbaPixels);
	SetFilters(texture, generateMipmaps);

	if (generateMipmaps)
	{
		GenerateMipmaps(newWidth, newHeight, rgbaPixels);
	}
}

void TextureLoader::UploadIndexed8(GLuint texture, int width, int height, const std::byte* pixels, const RGBPalette& palette, bool generateMipmaps, bool masked)
{
	//TODO: total size can be too large
	RGBPalette localPalette{palette};

	//Sets the mask color to black. This helps limit the bleedover effect caused by resizing and filtering
	if (masked)
	{
		localPalette.GetAlpha() = {0, 0, 0};
	}

	std::vector<std::byte> rgbaPixels;

	rgbaPixels.resize(width * height * 4);

	for (int i = 0; i < (width * height); ++i)
	{
		rgbaPixels[(i * 4) + 0] = std::byte{localPalette[std::to_integer<int>(pixels[i])].R};
		rgbaPixels[(i * 4) + 1] = std::byte{localPalette[std::to_integer<int>(pixels[i])].G};
		rgbaPixels[(i * 4) + 2] = std::byte{localPalette[std::to_integer<int>(pixels[i])].B};

		//For masked textures the last color in the table is the transparent color
		//Pixels with that color have their alpha value set to 0 to appear transparent
		if (masked && pixels[i] == std::byte{RGBPalette::AlphaIndex})
		{
			rgbaPixels[(i * 4) + 3] = std::byte{0x00};
		}
		else
		{
			rgbaPixels[(i * 4) + 3] = std::byte{0xFF};
		}
	}

	UploadRGBA8888(texture, width, height, rgbaPixels.data(), generateMipmaps, masked);
}

void TextureLoader::SetFilters(GLuint texture, bool hasMipmaps)
{
	_openglFunctions->glBindTexture(GL_TEXTURE_2D, texture);
	_openglFunctions->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, hasMipmaps ? _glMinFilter : _glMagFilter);
	_openglFunctions->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, _glMagFilter);
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

void TextureLoader::GenerateMipmaps(int width, int height, const std::byte* pixels)
{
	std::vector<std::byte> rescaledPixels{pixels, pixels + (width * height * 4)};

	int mipLevel = 0;

	while (width > 1 || height > 1)
	{
		const int rowLength = width * 4;

		width = std::max(1, width / 2);
		height = std::max(1, height / 2);

		ResizeMipmap(rescaledPixels, width, height, rowLength);

		_openglFunctions->glTexImage2D(
			GL_TEXTURE_2D, ++mipLevel, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rescaledPixels.data());
	}
}

void TextureLoader::ResizeMipmap(std::vector<std::byte>& pixels, int width, int height, int rowLength)
{
	auto src = reinterpret_cast<const std::uint8_t*>(pixels.data());
	auto dest = reinterpret_cast<std::uint8_t*>(pixels.data());

	// Average out 4 pixels into one.
	for (int y = 0; y < height; ++y, src += rowLength)
	{
		for (int x = 0; x < width; ++x, src += 8, dest += 4)
		{
			for (int i = 0; i < 4; ++i)
			{
				dest[i] = (src[i] + src[i + 4] + src[rowLength + i] + src[rowLength + i + 4]) / 4;
			}
		}
	}
}
}
