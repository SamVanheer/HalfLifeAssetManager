#ifndef CVAR_CBASECONCOMMAND_H
#define CVAR_CBASECONCOMMAND_H

#include "ConVarConstants.h"

namespace cvar
{
/**
*	Base class for all console commands.
*	Abstract.
*/
class CBaseConCommand
{
protected:
	/**
	*	Constructor. Creates a command with the given name.
	*	@param pszName Name of this command. Must be non-null and not empty.
	*	@param flags Flags. @see cvar::Flag::Flag
	*	@param pszHelpInfo Optional help info.
	*/
	CBaseConCommand( const char* const pszName, const Flags_t flags = Flag::NONE, const char* const pszHelpInfo = "" );

public:
	virtual ~CBaseConCommand() = 0;

	/**
	*	Gets the head of the command list.
	*/
	static CBaseConCommand* GetHead() { return m_pHead; }

	/**
	*	Gets the next command in the command list.
	*/
	CBaseConCommand* GetNext() const { return m_pNext; }

	/**
	*	Gets the name of this command.
	*/
	const char* GetName() const { return m_pszName; }

	/**
	*	Gets the help info that describes this command. May be empty.
	*/
	const char* GetHelpInfo() const { return m_pszHelpInfo; }

	Flags_t GetFlags() const { return m_Flags; }

	/**
	*	Gets the type of this command.
	*/
	virtual CommandType GetType() const  = 0;

private:
	static CBaseConCommand* m_pHead;
	CBaseConCommand* m_pNext;

	const char* const m_pszName;
	const char* const m_pszHelpInfo;
	const Flags_t m_Flags;

private:
	CBaseConCommand( const CBaseConCommand& ) = delete;
	CBaseConCommand& operator=( const CBaseConCommand& ) = delete;
};

inline CBaseConCommand::CBaseConCommand( const char* const pszName, const Flags_t flags, const char* const pszHelpInfo )
	: m_pNext( m_pHead )
	, m_pszName( pszName )
	, m_pszHelpInfo( pszHelpInfo ? pszHelpInfo : "" )
	, m_Flags( flags )
{
	m_pHead = this;
}

inline CBaseConCommand::~CBaseConCommand()
{
}
}

#endif //CVAR_CBASECONCOMMAND_H
