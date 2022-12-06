#pragma once

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
