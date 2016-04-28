#include <cassert>

#include "CConCommand.h"

namespace cvar
{
IConCommandHandler* const CConCommand::NULL_HANDLER = nullptr;

void CConCommand::SetCallbackFn( const CommandCallback callbackFn )
{
	assert( callbackFn );

	//If the function is null, this will leave the command in an undefined state. Solution: don't pass null functions.
	if( !callbackFn )
	{
		return;
	}

	m_CallbackFn = callbackFn;

	m_CallbackType = CallbackType::FUNCTION;
}

void CConCommand::SetHandler( IConCommandHandler* pCallbackObj )
{
	m_pCallbackObj = pCallbackObj;

	m_CallbackType = CallbackType::INTERFACE;
}
}