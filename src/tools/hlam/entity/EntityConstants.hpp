#pragma once

#include "core/shared/Utility.hpp"

namespace entity
{
typedef unsigned int Flags;

enum Flag : Flags
{
	FL_NONE = 0,

	/**
	*	Marks an entity as needing to be removed at the end of the frame.
	*/
	FL_KILLME = Bit(0),

	/**
	*	If set, the entity's think method is called every frame.
	*/
	FL_ALWAYSTHINK = Bit(1)
};
}
