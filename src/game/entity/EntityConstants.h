#ifndef GAME_ENTITY_ENTITYCONSTANTS_H
#define GAME_ENTITY_ENTITYCONSTANTS_H

namespace entity
{
const size_t INVALID_ENTITY_INDEX = -1;

enum DrawFlag
{
	DRAWF_NONE				= 0,
	DRAWF_WIREFRAME_ONLY	= 1 << 0,
};

typedef unsigned int DrawFlags_t;
}

#endif //GAME_ENTITY_ENTITYCONSTANTS_H
