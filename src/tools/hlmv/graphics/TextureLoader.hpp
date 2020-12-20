#pragma once

#include <utility>

#include <GL/glew.h>

#include "core/shared/Const.hpp"

namespace graphics
{
class TextureLoader final
{
public:
	TextureLoader();
	~TextureLoader();

	bool ShouldFilterTextures() const { return _filterTextures; }

	void SetFilterTextures(bool value)
	{
		_filterTextures = value;
	}

	bool ShouldResizeToPowerOf2() const { return _resizeToPowerOf2; }

	void SetResizeToPowerOf2(bool value)
	{
		_resizeToPowerOf2 = value;
	}

	void UploadRGBA8888(GLuint texture, int width, int height, const byte* rgbaPixels, bool masked);

	void UploadIndexed8(GLuint texture, int width, int height, const byte* pixels, const byte* palette, bool masked);

	void SetFilters(GLuint texture);

private:
	std::pair<int, int> AdjustImageDimensions(int width, int height) const;

private:
	bool _filterTextures{true};
	bool _resizeToPowerOf2{true};
};
}
