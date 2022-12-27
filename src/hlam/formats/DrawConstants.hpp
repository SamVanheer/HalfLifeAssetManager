#pragma once

#include <cstdint>

#include "utility/Utility.hpp"

namespace renderer
{
typedef uint32_t DrawFlags;

namespace DrawFlag
{
/**
*	Flags for drawing operations.
*/
enum DrawFlag : DrawFlags
{
	NONE = 0,

	/**
	*	Don't draw the model itself.
	*/
	NODRAW = 1 << 0,

	/**
	*	Draw wireframe overlay.
	*/
	WIREFRAME_OVERLAY = 1 << 1,

	/**
	*	Whether this is a view model
	*/
	IS_VIEW_MODEL = 1 << 2,

	DRAW_SHADOWS = 1 << 3, //! Whether to draw shadows

	FIX_SHADOW_Z_FIGHTING = 1 << 4, //! Whether to fix Shadow Z fighting

	DRAW_HITBOXES = 1 << 5,

	DRAW_BONES = 1 << 6,

	DRAW_ATTACHMENTS = 1 << 7,

	DRAW_EYE_POSITION = 1 << 8,

	DRAW_NORMALS = 1 << 9,
};
}
}
