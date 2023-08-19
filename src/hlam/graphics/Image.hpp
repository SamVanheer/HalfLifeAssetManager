#pragma once

#include <cassert>
#include <cstddef>
#include <cstring>
#include <vector>

namespace graphics
{
/**
*	@brief Container for 2D RGBA8888 image data.
*/
class Image final
{
public:
	static constexpr std::size_t BytesPerPixel = 4;

	Image() = default;

	Image(int width, int height, std::vector<std::byte>&& data)
		: _width(width)
		, _height(height)
		, _data(std::move(data))
	{
		assert(width >= 0);
		assert(height >= 0);
		assert(_data.size() == (width * height * BytesPerPixel));
	}

	Image(int width, int height, const std::byte* data)
		: Image(width, height, Convert(width, height, data))
	{
	}

	Image(const Image&) = default;
	Image& operator=(const Image&) = default;

	Image(Image&&) = default;
	Image& operator=(Image&&) = default;

	int GetWidth() const { return _width; }

	int GetHeight() const { return _height; }

	const std::vector<std::byte>& GetData() const { return _data; }

private:
	static std::vector<std::byte> Convert(int width, int height, const std::byte* data)
	{
		std::vector<std::byte> result;

		result.resize(width * height * BytesPerPixel);

		std::memcpy(result.data(), data, result.size());

		return result;
	}

private:
	int _width = 0;
	int _height = 0;
	std::vector<std::byte> _data;
};
}
