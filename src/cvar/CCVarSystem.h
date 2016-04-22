#ifndef CVAR_CCVARSYSTEM_H
#define CVAR_CCVARSYSTEM_H

#include <unordered_map>

#include "common/Utility.h"

#include "utility/CCommand.h"

#include "CConCommand.h"

namespace cvar
{
class CBaseConCommand;
class CCVar;

class CCvarSystem final : public IConCommandHandler
{
private:
	//The default hasher and equal comparator for const char* operate on the memory address, not the string itself.
	typedef std::unordered_map<const char*, CBaseConCommand*, Hash_C_String<const char*>, EqualTo_C_String<const char*>> Commands_t;

	/**
	*	Maximum size of the command buffer. This should be able to store 64 commands that use the maximum space available.
	*/
	static const size_t MAX_COMMAND_BUFFER = util::CCommand::MAX_LENGTH * 64;

public:
	CCvarSystem();
	~CCvarSystem();

	/**
	*	Returns whether the system has been initialized.
	*/
	bool IsInitialized() const { return m_bInitialized; }

	/**
	*	Initializes the system.
	*	@return true on success, false otherwise.
	*/
	bool Initialize();

	/**
	*	Shuts down the system.
	*/
	void Shutdown();

	/**
	*	Processes commands.
	*	Must be called every frame.
	*/
	void RunFrame();

	/**
	*	Adds the given command to the system. The command must not already be managed by the system, and must have a unique name.
	*	@param pCommand Command to add.
	*	@return true on success, false otherwise.
	*/
	bool AddCommand( CBaseConCommand* const pCommand );

	/**
	*	Removes the given command from the system, if it is managed by it.
	*/
	void RemoveCommand( CBaseConCommand* const pCommand );

	/**
	*	Removes a command by name.
	*/
	void RemoveCommand( const char* const pszName );

	/**
	*	Enqueues a command for execution.
	*/
	void Command( const char* const pszCommand );

	/**
	*	Executes the current command buffer.
	*/
	void Execute();

	void CVarChanged( const CCVar& cvar, const char* pszOldValue, float flOldValue );

private:
	void ProcessCommand( const util::CCommand& args );

	void HandleConCommand( const CConCommand& command, const util::CCommand& args ) override final;

private:
	bool m_bInitialized = false;

	Commands_t m_Commands;

	char m_szCommandBuffer[ MAX_COMMAND_BUFFER ];

	/**
	*	If set to true while executing commands, will suspend command execution until the next frame.
	*/
	bool m_bWait = false;

private:
	CCvarSystem( const CCvarSystem& ) = delete;
	CCvarSystem& operator=( const CCvarSystem& ) = delete;
};

CCvarSystem& cvars();
}

#endif //CVAR_CCVARSYSTEM_H
