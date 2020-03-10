#ifndef CVAR_CCVARSYSTEM_H
#define CVAR_CCVARSYSTEM_H

#include <unordered_map>
#include <vector>

#include "utility/StringUtils.h"

#include "utility/CCommand.h"

#include "cvar/ICVarSystem.h"

namespace cvar
{
class CCVarSystem final : public ICVarSystem, public IConCommandHandler
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
	using CVarArchiveCallback = void ( * )( void* pObject, const CCVar& cvar );

public:
	CCVarSystem();
	~CCVarSystem();

	bool IsInitialized() const override final { return m_bInitialized; }

	bool Initialize() override final;

	void Shutdown() override final;

	void RunFrame() override final;

	bool AddCommand( CBaseConCommand* const pCommand ) override final;

	void RemoveCommand( CBaseConCommand* const pCommand ) override final;

	void RemoveCommand( const char* const pszName ) override final;

	void Command( const char* const pszCommand ) override final;

	void Execute() override final;

	const CBaseConCommand* FindCommand( const char* const pszName ) const override final;

	CBaseConCommand* FindCommand( const char* const pszName ) override final;

private:
	const CCVar* GetCVarWarn( const char* const pszCVar ) const;

public:
	const char* GetCVarString( const char* const pszCVar ) const override final;

	float GetCVarFloat( const char* const pszCVar ) const override final;

	void SetCVarString( const char* const pszCVar, const char* const pszValue ) override final;

	void SetCVarFloat( const char* const pszCVar, const float flValue ) override final;

	void CVarChanged( CCVar& cvar, const char* pszOldValue, float flOldValue ) override final;

	void ArchiveCVars( const CVarArchiveCallback pCallback, void* pObject = nullptr ) override final;

	bool HasGlobalCVarHandler( ICVarHandler* pHandler ) const override final;

	bool InstallGlobalCVarHandler( ICVarHandler* pHandler ) override final;

	void RemoveGlobalCVarHandler( ICVarHandler* pHandler ) override final;

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
}

#endif //CVAR_CCVARSYSTEM_H
