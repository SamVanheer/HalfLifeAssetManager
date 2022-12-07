#pragma once

#include "utility/Utility.hpp"

namespace entity
{
typedef unsigned int Flags;

enum Flag : Flags
{
	FL_NONE = 0,

	/**
	*	If set, the entity's think method is called every frame.
	*/
	FL_ALWAYSTHINK = Bit(0)
};
}
