#ifndef CVAR_CONVARCONSTANTS_H
#define CVAR_CONVARCONSTANTS_H

#include <cstdint>

#include "common/Utility.h"

/**
*	@defgroup CVar Command variables that can be used system wide. Includes command execution for function like operations.
*	@{
*/
namespace cvar
{
/**
*	Defines the types of commands that can exist
*/
enum class CommandType : uint8_t
{
	FIRST		= 0,

	/**
	*	@see cvar::CConCommand
	*/
	COMMAND		= FIRST,

	/**
	*	@see cvar::CCVar
	*/
	CVAR,

	COUNT,
	LAST		= COUNT - 1
};

/**
*	Types of callbacks.
*/
enum class CallbackType : uint8_t
{
	FIRST		= 0,

	FUNCTION	= FIRST,
	INTERFACE,

	COUNT,
	LAST		= COUNT - 1
};

typedef uint32_t Flags_t;

namespace Flag
{
enum Flag : Flags_t
{
	NONE		= 0,
	ARCHIVE		= Bit( 0 )
};
}
}
/**@}*/

#endif //CVAR_CONVARCONSTANTS_H
