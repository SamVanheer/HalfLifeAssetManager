#pragma once

#include <cstddef>
#include <utility>

#include <GL/glew.h>

#include "graphics/Palette.hpp"

namespace graphics
{
enum class TextureFilter
{
	Point,
	Linear,

	First = Point,
	Last = Linear
};

enum class MipmapFilter
{
	None,
	Point,
	Linear,

	First = None,
	Last = Linear
};

class TextureLoader final
{
public:
	TextureLoader();
	~TextureLoader();

	TextureFilter GetMinFilter() const { return _minFilter; }

	TextureFilter GetMagFilter() const { return _magFilter; }

	MipmapFilter GetMipmapFilter() const { return _mipmapFilter; }

	void SetTextureFilters(TextureFilter minFilter, TextureFilter magFilter, MipmapFilter mipmapFilter);

	bool ShouldResizeToPowerOf2() const { return _resizeToPowerOf2; }

	void SetResizeToPowerOf2(bool value)
	{
		_resizeToPowerOf2 = value;
	}

	void UploadRGBA8888(GLuint texture, int width, int height, const std::byte* rgbaPixels, bool generateMipmaps, bool masked);

	void UploadIndexed8(GLuint texture, int width, int height, const std::byte* pixels, const RGBPalette& palette, bool generateMipmaps, bool masked);

	void SetFilters(GLuint texture, bool hasMipmaps);

private:
	std::pair<int, int> AdjustImageDimensions(int width, int height) const;

private:
	TextureFilter _minFilter{TextureFilter::Linear};
	TextureFilter _magFilter{TextureFilter::Linear};
	MipmapFilter _mipmapFilter{MipmapFilter::None};

	GLint _glMinFilter;
	GLint _glMagFilter;

	bool _resizeToPowerOf2{true};
};
}
