#pragma once

#include <array>
#include <cstddef>

#include "core/shared/Const.hpp"

/*
*	@file
*
*	Definitions for 24 and 32 bit 256 color palettes.
*/

namespace graphics
{
struct RGB24
{
	byte R = 0;
	byte G = 0;
	byte B = 0;

	RGB24() = default;

	RGB24(byte r, byte g, byte b)
		: R(r)
		, G(g)
		, B(b)
	{
	}

	RGB24(const RGB24&) = default;
	RGB24& operator=(const RGB24& other) = default;
};

struct RGBA32 : public RGB24
{
	byte A = 0;

	RGBA32() = default;

	RGBA32(byte r, byte g, byte b, byte a)
		: RGB24(r, g, b)
		, A(a)
	{
	}

	RGBA32(const RGB24& other)
		: RGB24(other)
	{
	}

	RGBA32(const RGBA32&) = default;

	RGBA32& operator=(const RGB24& other)
	{
		if (this != &other)
		{
			R = other.R;
			G = other.G;
			B = other.B;
		}

		return *this;
	}

	RGBA32& operator=(const RGBA32& other) = default;
};

/**
*	@brief Palette that stores 256 colors. Size and layout is identical to a raw palette.
*/
template<typename TColor>
struct Palette final
{
	using ColorType = TColor;

	/**
	*	@brief Number of entries.
	*/
	static constexpr std::size_t EntriesCount = 256;

	/**
	*	@brief Number of channels
	*/
	static constexpr std::size_t ChannelsCount = sizeof(TColor) / sizeof(byte);

	/**
	*	@brief The index in a palette where the alpha color is stored. Used for transparent textures.
	*/
	static constexpr std::size_t AlphaIndex = 255;

	Palette() noexcept = default;
	Palette(const Palette&) noexcept = default;
	Palette& operator=(const Palette&) noexcept = default;

	Palette(const std::array<TColor, EntriesCount>& palette) noexcept
		: Data(palette)
	{
	}

	const TColor& operator[](std::size_t index) const noexcept
	{
		return Data[index];
	}

	TColor& operator[](std::size_t index) noexcept
	{
		return Data[index];
	}

	std::size_t size() const noexcept { return Data.size(); }

	auto begin() const noexcept
	{
		return Data.begin();
	}

	auto begin() noexcept
	{
		return Data.begin();
	}

	auto end() const noexcept
	{
		return Data.end();
	}

	auto end() noexcept
	{
		return Data.end();
	}

	const TColor& GetAlpha() const noexcept
	{
		return Data[AlphaIndex];
	}

	TColor& GetAlpha() noexcept
	{
		return Data[AlphaIndex];
	}

	const byte* AsByteArray() const noexcept
	{
		return reinterpret_cast<const byte*>(Data.data());
	}

	byte* AsByteArray() noexcept
	{
		return reinterpret_cast<byte*>(Data.data());
	}

	std::size_t GetSizeInBytes() const noexcept
	{
		return sizeof(*this);
	}

	std::array<TColor, EntriesCount> Data{};
};

using RGBPalette = Palette<RGB24>;
using RGBAPalette = Palette<RGBA32>;
}
