#include <cassert>
#include <cctype>
#include <cstring>

#include "shared/Logging.h"

#include "utility/StringUtils.h"

#include "lib/LibInterface.h"

#include "CBaseConCommand.h"
#include "CConCommand.h"
#include "CCVar.h"

#include "CVar.h"

#include "CCVarSystem.h"

namespace cvar
{
namespace
{
static CCVarSystem g_CVars;

static CConCommand g_Wait( "wait", &g_CVars, Flag::NONE, "Suspends command execution for the current frame" );

static CCVar g_ShowWait( "showwait", CCVarArgsBuilder().FloatValue( 0 ).HelpInfo( "If non-zero, outputs text every time a wait command is processed" ) );

static CConCommand g_Find( "find", &g_CVars, Flag::NONE, "Finds commands by searching by name and through help info" );
}

REGISTER_INTERFACE_GLOBAL( ICVARSYSTEM_NAME, CCVarSystem, &g_CVars );

CCVarSystem::CCVarSystem()
{
	memset( m_szCommandBuffer, 0, sizeof( m_szCommandBuffer ) );

	g_pCVar = this;
}

CCVarSystem::~CCVarSystem()
{
}

bool CCVarSystem::Initialize()
{
	CBaseConCommand* pCommand = CBaseConCommand::GetHead();

	while( pCommand )
	{
		AddCommand( pCommand );

		pCommand = pCommand->GetNext();
	}

	m_bInitialized = true;

	return true;
}

void CCVarSystem::Shutdown()
{
	m_bInitialized = false;

	m_Commands.clear();
}

void CCVarSystem::RunFrame()
{
	Execute();
}

bool CCVarSystem::AddCommand( CBaseConCommand* const pCommand )
{
	assert( pCommand );

	if( strpbrk( pCommand->GetName(), " \t\"\';" ) )
	{
		Warning( "Attempted to add command with invalid characters in its name \"%s\"!\n", pCommand->GetName() );
		return false;
	}

	{
		auto it = m_Commands.find( pCommand->GetName() );

		//Duplicate command; check if they're compatible or not.
		if( it != m_Commands.end() )
		{
			auto pParent = it->second;

			if( pParent->GetType() != pCommand->GetType() )
			{
				Warning( "Attempted to add duplicate command \"%s\" with different types!\n", pCommand->GetName() );
				return false;
			}

			if( pParent == pCommand )
			{
				Warning( "Attempted to add command \"%s\" twice!\n", pCommand->GetName() );
				return false;
			}

			switch( pCommand->GetType() )
			{
			case CommandType::COMMAND:
				{
					auto pConCommand = static_cast<CConCommand*>( pCommand );
					auto pParentCommand = static_cast<CConCommand*>( pParent );

					if( pConCommand->GetCallbackType() != pParentCommand->GetCallbackType() )
					{
						Warning( "ConCommand \"%s\" has child with different callback type!\n", pCommand->GetName() );
						return false;
					}

					if( pConCommand->GetCallbackType() == CallbackType::FUNCTION ? 
						( pConCommand->GetCallbackFn() != pParentCommand->GetCallbackFn() ) : 
						( pConCommand->GetHandler() != pParentCommand->GetHandler() ) )
					{
						Warning( "ConCommand \"%s\" has child with different callback!\n", pCommand->GetName() );
						return false;
					}

					break;
				}

			case CommandType::CVAR:
				{
					auto pConVar = static_cast<CCVar*>( pCommand );
					auto pParentVar = static_cast<CCVar*>( pParent );

					if( pConVar->GetCallbackType() != pParentVar->GetCallbackType() )
					{
						Warning( "ConVar \"%s\" has child with different callback type!\n", pCommand->GetName() );
						return false;
					}

					if( pConVar->GetCallbackType() == CallbackType::FUNCTION ?
						( pConVar->GetCallbackFn() != pParentVar->GetCallbackFn() ) :
						( pConVar->GetHandler() != pParentVar->GetHandler() ) )
					{
						Warning( "ConVar \"%s\" has child with different callback!\n", pCommand->GetName() );
						return false;
					}

					//Bind child to parent so everything uses the same one.
					pConVar->m_pParent = pParentVar;

					break;
				}

			default:
				{
					Warning( "Unknown command type %u for command \"%s\"!\n", pCommand->GetType(), pCommand->GetName() );
					return false;
				}
			}

			return true;
		}
	}

	auto it = m_Commands.insert( std::make_pair( pCommand->GetName(), pCommand ) );

	return it.second;
}

void CCVarSystem::RemoveCommand( CBaseConCommand* const pCommand )
{
	if( !pCommand )
	{
		return;
	}

	auto it = m_Commands.find( pCommand->GetName() );

	if( it == m_Commands.end() )
	{
		return;
	}

	if( it->second != pCommand )
	{
		Warning( "CCvarSystem::RemoveCommand: Called with command \"%s\", but a different memory address!\n", pCommand->GetName() );
		return;
	}

	m_Commands.erase( it );
}

void CCVarSystem::RemoveCommand( const char* const pszName )
{
	assert( pszName );

	if( !( *pszName ) )
	{
		return;
	}

	auto it = m_Commands.find( pszName );

	if( it == m_Commands.end() )
	{
		return;
	}

	m_Commands.erase( pszName );
}

void CCVarSystem::Command( const char* const pszCommand )
{
	assert( pszCommand );

	if( !( *pszCommand ) )
		return;

	const size_t uiCommandLength = strlen( pszCommand );

	const size_t uiBufferLength = strlen( m_szCommandBuffer );

	//Handle overflow here so we don't miss anything.
	if( uiBufferLength >= MAX_COMMAND_BUFFER || 
		uiBufferLength + uiCommandLength >= MAX_COMMAND_BUFFER ||
		uiBufferLength + uiCommandLength < uiBufferLength )
	{
		//TODO: replace with FatalError
		Error( "Command buffer full!\n" );
		return;
	}

	strcpy( m_szCommandBuffer + uiBufferLength, pszCommand );

	if( pszCommand[ uiCommandLength - 1 ] != ';' &&
		pszCommand[ uiCommandLength - 1 ] != '\n' )
	{
		//The buffer is full. EOF is treated as the end of a command, so this will work.
		if( uiBufferLength + uiCommandLength + 1 >= MAX_COMMAND_BUFFER )
			return;

		//Append a newline if there isn't one already.
		m_szCommandBuffer[ uiBufferLength + uiCommandLength ] = '\n';
	}
}

void CCVarSystem::Execute()
{
	//The wait is over.
	m_bWait = false;

	//Process the command buffer.
	char* pszCommand = m_szCommandBuffer;

	char* pszBufferEnd = m_szCommandBuffer + strlen( m_szCommandBuffer );

	util::CCommand command;

	while( pszCommand < pszBufferEnd )
	{
		//Skip whitespace
		while( isspace( *pszCommand ) )
		{
			++pszCommand;
		}

		if( !( *pszCommand ) )
			break;

		//Found a command. Find the end, then parse it in and process it.
		char* pszCommandEnd = pszCommand + 1;

		while( *pszCommandEnd && *pszCommandEnd != '\n' && *pszCommandEnd != ';' )
		{
			++pszCommandEnd;
		}

		*pszCommandEnd = '\0';

		const bool bResult = command.Initialize( pszCommand );

		pszCommand = pszCommandEnd + 1;

		if( bResult )
		{
			//Process it.
			ProcessCommand( command );

			//Processed a wait command.
			if( m_bWait )
			{
				m_bWait = false;

				if( g_ShowWait.GetBool() )
				{
					Message( "Waiting\n" );
				}

				break;
			}
		}
		else
		{
			Error( "Failed to initialize command with contents \"%s\"\n", pszCommand );
		}
	}

	//There are commands left in the buffer.
	if( pszCommand < pszBufferEnd )
	{
		const size_t uiLength = pszBufferEnd - pszCommand;
		memmove( m_szCommandBuffer, pszCommand, uiLength );

		//Avoid reprocessing the commands again.
		m_szCommandBuffer[ uiLength ] = '\0';
	}
	else
	{
		memset( m_szCommandBuffer, 0, sizeof( m_szCommandBuffer ) );
	}
}

const CBaseConCommand* CCVarSystem::FindCommand( const char* const pszName ) const
{
	assert( pszName );

	if( !( *pszName ) )
		return nullptr;

	auto it = m_Commands.find( pszName );

	if( it == m_Commands.end() )
	{
		return nullptr;
	}

	return it->second;
}

CBaseConCommand* CCVarSystem::FindCommand( const char* const pszName )
{
	return const_cast<CBaseConCommand*>( const_cast<const CCVarSystem* const>( this )->FindCommand( pszName ) );
}

const CCVar* CCVarSystem::GetCVarWarn( const char* const pszCVar ) const
{
	assert( pszCVar );

	auto pCommand = FindCommand( pszCVar );

	if( !pCommand )
	{
		Warning( "Unknown CVar \"%s\", ignoring\n", pszCVar );
		return nullptr;
	}

	if( pCommand->GetType() != CommandType::CVAR )
	{
		Warning( "Command \"%s\" is not a CVar\n", pszCVar );
		return nullptr;
	}

	return static_cast<const CCVar*>( pCommand );
}

const char* CCVarSystem::GetCVarString( const char* const pszCVar ) const
{
	const CCVar* pCVar = GetCVarWarn( pszCVar );

	if( !pCVar )
		return "";

	return pCVar->GetString();
}

float CCVarSystem::GetCVarFloat( const char* const pszCVar ) const
{
	const CCVar* pCVar = GetCVarWarn( pszCVar );

	if( !pCVar )
		return 0;

	return pCVar->GetFloat();
}

void CCVarSystem::SetCVarString( const char* const pszCVar, const char* const pszValue )
{
	assert( pszValue );

	CCVar* pCVar = const_cast<CCVar*>( GetCVarWarn( pszCVar ) );

	if( !pCVar )
		return;

	pCVar->SetString( pszValue );
}

void CCVarSystem::SetCVarFloat( const char* const pszCVar, const float flValue )
{
	CCVar* pCVar = const_cast<CCVar*>( GetCVarWarn( pszCVar ) );

	if( !pCVar )
		return;

	pCVar->SetFloat( flValue );
}

void CCVarSystem::CVarChanged( CCVar& cvar, const char* pszOldValue, float flOldValue )
{
	//Don't print this on init.
	if( !IsInitialized() )
		return;

	Message( "\"%s\" changed to \"%s\"\n", cvar.GetName(), cvar.GetString() );

	CallGlobalCVarHandlers( cvar, pszOldValue, flOldValue );
}

void CCVarSystem::ArchiveCVars( const CVarArchiveCallback pCallback, void* pObject )
{
	assert( pCallback );

	for( const auto command : m_Commands )
	{
		if( command.second->GetType() != CommandType::CVAR )
			continue;

		const CCVar& cvar = static_cast<const CCVar&>( *command.second );

		if( cvar.GetFlags() & Flag::ARCHIVE )
		{
			pCallback( pObject, cvar );
		}
	}
}

void CCVarSystem::ProcessCommand( const util::CCommand& args )
{
	assert( args.IsValid() );

	Message( "] %s\n", args.GetCommandString() );

	const char* const pszName = args.Arg( 0 );

	auto it = m_Commands.find( pszName );

	if( it == m_Commands.end() )
	{
		Message( "Unknown command: %s\n", pszName );
		return;
	}

	CBaseConCommand* const pCommand = it->second;

	assert( pCommand );

	switch( pCommand->GetType() )
	{
	case CommandType::COMMAND:
		{
			CConCommand* const pConCommand = static_cast<CConCommand* const>( pCommand );

			switch( pConCommand->GetCallbackType() )
			{
			case CallbackType::FUNCTION:
				{
					auto callback = pConCommand->GetCallbackFn();

					assert( callback );

					callback( args );

					break;
				}

			case CallbackType::INTERFACE:
				{
					auto pObj = pConCommand->GetHandler();

					if( pObj )
					{
						pObj->HandleConCommand( *pConCommand, args );
					}
					else
					{
						Warning( "Command \"%s\" has handler callback, but handler is null!\n", pszName );
					}

					break;
				}

			default:
				{
					Error( "Unknown callback type \"%d\" for command \"%s\"!\n", pConCommand->GetCallbackType(), pszName );
					break;
				}
			}

			break;
		}

	case CommandType::CVAR:
		{
			CCVar* const pCVar = static_cast<CCVar* const>( pCommand );

			if( args.ArgC() != 2 )
			{
				Message( "\"%s\" is \"%s\"\n", pszName, pCVar->GetString() );

				break;
			}

			pCVar->SetString( args.Arg( 1 ) );

			break;
		}

	default:
		{
			Error( "Unknown command type \"%d\" for command \"%s\"!\n", pCommand->GetType(), pszName );
			break;
		}
	}
}

void CCVarSystem::HandleConCommand( const CConCommand& command, const util::CCommand& args )
{
	const char* const pszName = args.Arg( 0 );

	if( strcmp( pszName, "wait" ) == 0 )
	{
		m_bWait = true;
	}
	else if( strcmp( pszName, "find" ) == 0 )
	{
		if( args.ArgC() < 2 )
		{
			Message( "Usage: find <token>\n" );

			return;
		}

		const char* const pszToken = args.Arg( 1 );

		for( auto it = m_Commands.begin(), end = m_Commands.end(); it != end; ++it )
		{
			const CBaseConCommand* const pCommand = it->second;

			if( UTIL_TokenMatches( pCommand->GetName(), pszToken ) || UTIL_TokenMatches( pCommand->GetHelpInfo(), pszToken ) )
			{
				Message( "%s: %s\n", pCommand->GetName(), pCommand->GetHelpInfo() );
			}
		}
	}
}

bool CCVarSystem::HasGlobalCVarHandler( ICVarHandler* pHandler ) const
{
	if( !pHandler )
		return false;

	return std::find( m_GlobalCVarHandlers.begin(), m_GlobalCVarHandlers.end(), pHandler ) != m_GlobalCVarHandlers.end();
}

bool CCVarSystem::InstallGlobalCVarHandler( ICVarHandler* pHandler )
{
	assert( pHandler );

	if( HasGlobalCVarHandler( pHandler ) )
		return true;

	m_GlobalCVarHandlers.push_back( pHandler );

	return true;
}

void CCVarSystem::RemoveGlobalCVarHandler( ICVarHandler* pHandler )
{
	if( !pHandler )
		return;

	auto it = std::find( m_GlobalCVarHandlers.begin(), m_GlobalCVarHandlers.end(), pHandler );

	if( it == m_GlobalCVarHandlers.end() )
	{
		return;
	}

	m_GlobalCVarHandlers.erase( it );
}

void CCVarSystem::CallGlobalCVarHandlers( CCVar& cvar, const char* pszOldValue, float flOldValue )
{
	for( auto handler : m_GlobalCVarHandlers )
	{
		handler->HandleCVar( cvar, pszOldValue, flOldValue );
	}
}
}