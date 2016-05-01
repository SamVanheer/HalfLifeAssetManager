#ifndef ENGINE_SHARED_SPRITE_SPRITECONSTANTS_H
#define ENGINE_SHARED_SPRITE_SPRITECONSTANTS_H

#include <cstdint>

#include "shared/Utility.h"

/**
*	@ingroup Sprite
*
*	@{
*/

namespace sprite
{
typedef uint32_t DrawFlags_t;

/**
*	Flags for sprite drawing.
*/
enum DrawFlag : DrawFlags_t
{
	DRAWF_NONE = 0,

	/**
	*	Don't draw the sprite itself.
	*/
	DRAWF_NODRAW				= Bit( 0 ),

	/**
	*	Draw a wireframe overlay.
	*/
	DRAWF_WIREFRAME_OVERLAY		= Bit( 1 )
};
}

/** @} */

#endif //ENGINE_SHARED_SPRITE_SPRITECONSTANTS_H