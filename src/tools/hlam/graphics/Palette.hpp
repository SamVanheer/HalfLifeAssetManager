#pragma once

/*
*	Definitions for 24 bit 256 color palettes.
*/

/**
*	Number of entries.
*/
const size_t PALETTE_ENTRIES = 256;

/**
*	Number of channels (RGB)
*/
const size_t PALETTE_CHANNELS = 3;

/**
*	Total size of a palette, in bytes.
*/
const size_t PALETTE_SIZE = PALETTE_ENTRIES * PALETTE_CHANNELS;

/**
*	The index in a palette where the alpha color is stored. Used for transparent textures.
*/
const size_t PALETTE_ALPHA_INDEX = 255 * PALETTE_CHANNELS;
