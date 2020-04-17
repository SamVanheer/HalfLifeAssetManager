#ifndef ENGINE_RENDERER_DRAWCONSTANTS_H
#define ENGINE_RENDERER_DRAWCONSTANTS_H

#include <cstdint>

#include "shared/Utility.h"

namespace renderer
{
typedef uint32_t DrawFlags_t;

namespace DrawFlag
{
/**
*	Flags for drawing operations.
*/
enum DrawFlag : DrawFlags_t
{
	NONE				= 0,

	/**
	*	Don't draw the model itself.
	*/
	NODRAW				= Bit( 0 ),

	/**
	*	Draw wireframe overlay.
	*/
	WIREFRAME_OVERLAY	= Bit( 1 ),

	/**
	*	Whether this is a view model
	*/
	IS_VIEW_MODEL		= Bit( 2 ),

	DRAW_SHADOWS		= Bit(3), //! Whether to draw shadows

	FIX_SHADOW_Z_FIGHTING = Bit(4) //! Whether to fix Shadow Z fighting
};
}
}

#endif //ENGINE_RENDERER_DRAWCONSTANTS_H