#ifndef GAME_STUDIOMODEL_STUDIOMODELCONSTANTS_H
#define GAME_STUDIOMODEL_STUDIOMODELCONSTANTS_H

#include <cstdint>

#include "common/Utility.h"

namespace studiomodel
{
typedef uint32_t DrawFlags_t;

/**
*	Flags for studio model drawing.
*/
enum DrawFlag : DrawFlags_t
{
	DRAWF_NONE				= 0,

	/**
	*	Only draw wireframe
	*	TODO: should be turned into 2 flags: don't draw model, draw wireframe overlay
	*/
	DRAWF_WIREFRAME_ONLY	= Bit( 0 )
};
}

#endif //GAME_STUDIOMODEL_STUDIOMODELCONSTANTS_H