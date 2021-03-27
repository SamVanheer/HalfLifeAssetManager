#pragma once

/*
*	@file
*
*	Definitions for 24 bit 256 color palettes.
*/

namespace graphics
{
/**
*	@brief Number of entries.
*/
constexpr size_t PALETTE_ENTRIES = 256;

/**
*	@brief Number of channels (RGB)
*/
constexpr size_t PALETTE_CHANNELS = 3;

/**
*	@brief Total size of a palette, in bytes.
*/
constexpr size_t PALETTE_SIZE = PALETTE_ENTRIES * PALETTE_CHANNELS;

/**
*	@brief The index in a palette where the alpha color is stored. Used for transparent textures.
*/
constexpr size_t PALETTE_ALPHA_INDEX = 255 * PALETTE_CHANNELS;
}
