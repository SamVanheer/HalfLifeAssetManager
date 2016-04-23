#ifndef GAME_ENTITY_ENTITYCONSTANTS_H
#define GAME_ENTITY_ENTITYCONSTANTS_H

#include "common/Utility.h"

namespace entity
{
const size_t INVALID_ENTITY_INDEX = -1;

typedef unsigned int DrawFlags_t;

enum DrawFlag : DrawFlags_t
{
	DRAWF_NONE				= 0,

	/**
	*	Only draw wireframe overlay.
	*/
	DRAWF_WIREFRAME_ONLY	= Bit<DrawFlags_t>( 0 ),
};

typedef unsigned int Flags_t;

enum Flag : Flags_t
{
	FL_NONE = 0
};
}

#endif //GAME_ENTITY_ENTITYCONSTANTS_H
