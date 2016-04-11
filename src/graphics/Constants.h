#ifndef GRAPHICS_CONSTANTS_H
#define GRAPHICS_CONSTANTS_H

/*
*	Available render modes
*/
enum class RenderMode
{
	INVALID = -1,
	FIRST = 0,

	WIREFRAME = FIRST,
	FLAT_SHADED,
	SMOOTH_SHADED,
	TEXTURE_SHADED,

	COUNT,
	LAST = COUNT - 1 //Must be last
};

#endif //GRAPHICS_CONSTANTS_H