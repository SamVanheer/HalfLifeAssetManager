#ifndef CVAR_CCVARSYSTEM_H
#define CVAR_CCVARSYSTEM_H

#include <unordered_map>
#include <vector>

#include "common/Utility.h"

#include "utility/CCommand.h"

#include "CBaseConCommand.h"
#include "CConCommand.h"
#include "CCVar.h"

namespace cvar
{
class CCVarSystem final : public IConCommandHandler
{
private:
	//The default hasher and equal comparator for const char* operate on the memory address, not the string itself.
	typedef std::unordered_map<const char*, CBaseConCommand*, Hash_C_String<const char*>, EqualTo_C_String<const char*>> Commands_t;

	typedef std::vector<ICVarHandler*> GlobalCVarHandlers_t;

	/**
	*	Maximum size of the command buffer. This should be able to store 64 commands that use the maximum space available.
	*/
	static const size_t MAX_COMMAND_BUFFER = util::CCommand::MAX_LENGTH * 64;

public:
	CCVarSystem();
	~CCVarSystem();

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

	/**
	*	Finds a command by name.
	*/
	CBaseConCommand* FindCommand( const char* const pszName );

	void CVarChanged( CCVar& cvar, const char* pszOldValue, float flOldValue );

	//TODO: move this to its own class.
	/**
	*	Returns whether the given handler is currently installed.
	*/
	bool HasGlobalCVarHandler( ICVarHandler* pHandler ) const;

	/**
	*	Installs a global cvar handler.
	*	@param pHandler Handler to install.
	*	@return true on success, false otherwise.
	*/
	bool InstallGlobalCVarHandler( ICVarHandler* pHandler );

	/**
	*	Removes a global cvar handler.
	*	@param pHandler Handler to remove.
	*/
	void RemoveGlobalCVarHandler( ICVarHandler* pHandler );

private:
	void ProcessCommand( const util::CCommand& args );

	void HandleConCommand( const CConCommand& command, const util::CCommand& args ) override final;

	void CallGlobalCVarHandlers( CCVar& cvar, const char* pszOldValue, float flOldValue );

private:
	bool m_bInitialized = false;

	Commands_t m_Commands;

	GlobalCVarHandlers_t m_GlobalCVarHandlers;

	char m_szCommandBuffer[ MAX_COMMAND_BUFFER ];

	/**
	*	If set to true while executing commands, will suspend command execution until the next frame.
	*/
	bool m_bWait = false;

private:
	CCVarSystem( const CCVarSystem& ) = delete;
	CCVarSystem& operator=( const CCVarSystem& ) = delete;
};

CCVarSystem& cvars();
}

#endif //CVAR_CCVARSYSTEM_H
