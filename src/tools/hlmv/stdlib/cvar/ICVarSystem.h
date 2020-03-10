#ifndef CVAR_ICVARSYSTEM_H
#define CVAR_ICVARSYSTEM_H

/**
* @defgroup CVar
*
*	@{
*/

namespace cvar
{
class CBaseConCommand;
class CCVar;

class ICVarHandler;

class ICVarSystem
{
public:
	using CVarArchiveCallback = void( *)( void* pObject, const CCVar& cvar );

public:
	virtual ~ICVarSystem() = 0 {}

	/**
	*	@return Whether the system has been initialized.
	*/
	virtual bool IsInitialized() const = 0;

	/**
	*	Initializes the system.
	*	@return true on success, false otherwise.
	*/
	virtual bool Initialize() = 0;

	/**
	*	Shuts down the system.
	*/
	virtual void Shutdown() = 0;

	/**
	*	Processes commands.
	*	Must be called every frame.
	*/
	virtual void RunFrame() = 0;

	/**
	*	Adds the given command to the system. The command must not already be managed by the system, and must have a unique name.
	*	@param pCommand Command to add.
	*	@return true on success, false otherwise.
	*/
	virtual bool AddCommand( CBaseConCommand* const pCommand ) = 0;

	/**
	*	Removes the given command from the system, if it is managed by it.
	*/
	virtual void RemoveCommand( CBaseConCommand* const pCommand ) = 0;

	/**
	*	Removes a command by name.
	*/
	virtual void RemoveCommand( const char* const pszName ) = 0;

	/**
	*	Enqueues a command for execution.
	*/
	virtual void Command( const char* const pszCommand ) = 0;

	/**
	*	Executes the current command buffer.
	*/
	virtual void Execute() = 0;

	/**
	*	Finds a command by name.
	*/
	virtual const CBaseConCommand* FindCommand( const char* const pszName ) const = 0;

	/**
	*	Finds a command by name.
	*/
	virtual CBaseConCommand* FindCommand( const char* const pszName ) = 0;

	virtual const char* GetCVarString( const char* const pszCVar ) const = 0;

	virtual float GetCVarFloat( const char* const pszCVar ) const = 0;

	virtual void SetCVarString( const char* const pszCVar, const char* const pszValue ) = 0;

	virtual void SetCVarFloat( const char* const pszCVar, const float flValue ) = 0;

	virtual void CVarChanged( CCVar& cvar, const char* pszOldValue, float flOldValue ) = 0;

	/**
	*	Enumerates all CVars and calls pCallback for all CVars marked with the ARCHIVE flag.
	*/
	virtual void ArchiveCVars( const CVarArchiveCallback pCallback, void* pObject = nullptr ) = 0;

	/**
	*	Returns whether the given handler is currently installed.
	*/
	virtual bool HasGlobalCVarHandler( ICVarHandler* pHandler ) const = 0;

	/**
	*	Installs a global cvar handler.
	*	@param pHandler Handler to install.
	*	@return true on success, false otherwise.
	*/
	virtual bool InstallGlobalCVarHandler( ICVarHandler* pHandler ) = 0;

	/**
	*	Removes a global cvar handler.
	*	@param pHandler Handler to remove.
	*/
	virtual void RemoveGlobalCVarHandler( ICVarHandler* pHandler ) = 0;
};
}

/** @} */

#endif //CVAR_ICVARSYSTEM_H
