#pragma once

#include <cstdint>

using RenderPasses = std::uint32_t;

namespace RenderPass
{
/**
*	@brief Used with ::RenderPasses to specify which render passes a renderable object is a part of.
*/
enum RenderPass : RenderPasses
{
	None = 0,

	/**
	*	@brief Renders background imagery.
	*/
	Background = 1 << 0,

	/**
	*	@brief Renders the scene itself.
	*/
	Standard = 1 << 1,

	/**
	*	@brief For 3D objects that need to render on top of or around the scene.
	*/
	Overlay3D = 1 << 2,
};
}

/**
*	Available render modes
*/
enum class RenderMode
{
	FIRST = 0,

	WIREFRAME = FIRST,
	FLAT_SHADED,
	SMOOTH_SHADED,
	TEXTURE_SHADED,

	COUNT,
	LAST = COUNT - 1 //Must be last
};

const char* RenderModeToString( const RenderMode renderMode );
