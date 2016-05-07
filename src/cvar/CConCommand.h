#ifndef CVAR_CCONCOMMAND_H
#define CVAR_CCONCOMMAND_H

#include <cstdint>

#include "CBaseConCommand.h"

namespace util
{
class CCommand;
}

/**
*	@ingroup CVar
*	@{
*/
namespace cvar
{
class CConCommand;

/**
*	Callback for command execution.
*/
using CommandCallback = void ( * )( const util::CCommand& args );

/**
*	Interface for objects that want to handle command execution.
*/
class IConCommandHandler
{
public:
	virtual ~IConCommandHandler() = 0;

	/**
	*	Called when a command is being executed.
	*	@param command Command being executed.
	*	@param args Arguments.
	*/
	virtual void HandleConCommand( const CConCommand& command, const util::CCommand& args ) = 0;
};

inline IConCommandHandler::~IConCommandHandler()
{
}

/**
*	Command that can be executed through the console. Supports both functions and objects.
*/
class CConCommand : public CBaseConCommand
{
public:
	/**
	*	Convenient constant to pass to the constructor to resolve overload ambiguity.
	*/
	static IConCommandHandler* const NULL_HANDLER;

public:
	/**
	*	Creates a command with the given name and a function to call on execution.
	*	@param pszName Command name.
	*	@param callbackFn Function to call on execution. Must be non-null.
	*	@param flags Flags.
	*	@param pszHelpInfo Help info.
	*	@see CBaseConCommand::CBaseConCommand( const char* const pszName, const char* const pszHelpInfo )
	*/
	CConCommand( const char* const pszName, const CommandCallback callbackFn, const Flags_t flags = Flag::NONE, const char* const pszHelpInfo = "" );

	/**
	*	Creates a command with the given name and a function to call on execution.
	*	@param pszName Command name.
	*	@param pCallbackObj Object to call on execution. May be null. @see SetCommandHandler
	*	@param flags Flags.
	*	@param pszHelpInfo Help info.
	*	@see CBaseConCommand::CBaseConCommand( const char* const pszName, const char* const pszHelpInfo )
	*/
	CConCommand( const char* const pszName, IConCommandHandler* pCallbackObj, const Flags_t flags = Flag::NONE, const char* const pszHelpInfo = "" );

	virtual CommandType GetType() const override { return CommandType::COMMAND; }

	/**
	*	Returns whether this command is valid. Valid commands have a callback that can be invoked.
	*/
	bool IsValid() const;

	/**
	*	Gets the callback type. If the callback was an object, and was set to null, this will still return CallbackType::INTERFACE, but GetCommandHandler will return null.
	*/
	CallbackType GetCallbackType() const { return m_CallbackType; }

	/**
	*	Gets the callback as a function. Only valid if GetCallbackType == CallbackType::FUNCTION.
	*/
	CommandCallback GetCallbackFn() const { return m_CallbackFn; }

	/**
	*	Sets the callback as a function.
	*	@param callbackFn Function to set. Must be non-null.
	*/
	void SetCallbackFn( const CommandCallback callbackFn );

	/**
	*	Gets the callback as an object. Only valid if GetCallbackType == CallbackType::INTERFACE.
	*/
	IConCommandHandler* GetHandler() const { return m_pCallbackObj; }

	/**
	*	Sets the callback as an object.
	*	@param pCallbackObj Object to set. Can be null, but it is generally advised to set a valid object at all times. A warning is printed if the command is invoked with a null handler.
	*/
	void SetHandler( IConCommandHandler* pCallbackObj );

private:

	union
	{
		CommandCallback			m_CallbackFn;
		IConCommandHandler*		m_pCallbackObj;
	};

	CallbackType m_CallbackType;

private:
	CConCommand( const CConCommand& ) = delete;
	CConCommand& operator=( const CConCommand& ) = delete;
};

inline CConCommand::CConCommand( const char* const pszName, const CommandCallback callbackFn, const Flags_t flags, const char* const pszHelpInfo )
	: CBaseConCommand( pszName, flags, pszHelpInfo )
{
	SetCallbackFn( callbackFn );
}

inline CConCommand::CConCommand( const char* const pszName, IConCommandHandler* pCallbackObj, const Flags_t flags, const char* const pszHelpInfo )
	: CBaseConCommand( pszName, flags, pszHelpInfo )
{
	SetHandler( pCallbackObj );
}

inline bool CConCommand::IsValid() const
{
	switch( m_CallbackType )
	{
	case CallbackType::FUNCTION:		return m_CallbackFn != nullptr;
	case CallbackType::INTERFACE:	return m_pCallbackObj != nullptr;

		//Should never get here, but just in case.
	default:							return false;
	}
}
}

/**@}*/

#endif //CVAR_CCONCOMMAND_H