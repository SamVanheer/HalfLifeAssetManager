#include <cassert>
#include <cstdlib>
#include <cstring>

#include "utility/Tokenization.hpp"

#include "CCommand.hpp"

namespace util
{
namespace
{
static const char* GetArg_ArgV( void* pData, const int iIndex )
{
	return reinterpret_cast<char**>( pData )[ iIndex ];
}

static const char* GetArg_CCommand( void* pData, const int iIndex )
{
	return reinterpret_cast<CCommand*>( pData )->Arg( iIndex );
}
}

CCommand::CCommand()
{
	Reset();
}

CCommand::CCommand( const int iArgc, char** ppArgV )
{
	Initialize( iArgc, ppArgV );
}

bool CCommand::Initialize( const int iArgc, char** ppArgV )
{
	assert( iArgc > 0 );
	assert( ppArgV != nullptr );

	Reset();

	return Initialize( iArgc, ppArgV, &GetArg_ArgV );
}

bool CCommand::Initialize( const int iArgc, void* pData, const GetArg getArg )
{
	m_iArgc = iArgc;

	char* pszCmdStrBuffer = m_szCommandString;

	char* pszArgVBuffer = m_szArgsBuffer;

	for( int iIndex = 0; iIndex < m_iArgc; ++iIndex )
	{
		const char* pszToken = getArg( pData, iIndex );
		const size_t uiLength = strlen( pszToken );

		const bool bContainsSpace = strchr( pszToken, ' ' ) != nullptr;

		if( bContainsSpace && ( MAX_LENGTH - ( pszCmdStrBuffer - m_szCommandString ) ) > 1 )
			*pszCmdStrBuffer++ = '\"';

		if( static_cast<size_t>( MAX_LENGTH - ( pszCmdStrBuffer - m_szCommandString ) ) > uiLength )
		{
			strncpy( pszCmdStrBuffer, pszToken, uiLength + 1 );
			pszCmdStrBuffer += uiLength;
		}
		else //Token too large, handle error later
			m_iArgc = 0;

		if( bContainsSpace && ( MAX_LENGTH - ( pszCmdStrBuffer - m_szCommandString ) ) > 1 )
			*pszCmdStrBuffer++ = '\"';

		if( iIndex < m_iArgc - 1 && ( MAX_LENGTH - ( pszCmdStrBuffer - m_szCommandString ) ) > 1 )
			*pszCmdStrBuffer++ = ' ';

		//The command string buffer could not contain the entire string, abort
		if( m_iArgc == 0 || ( pszCmdStrBuffer - m_szCommandString ) <= 0 )
		{
			m_iArgc = 0;
			break;
		}

		//If the command string isn't overflowed yet, then the argV buffer can't be overflowed either
		m_pArgV[ iIndex ] = pszArgVBuffer;
		strncpy( pszArgVBuffer, pszToken, uiLength + 1 );

		//Don't forget the null terminator for each argument
		pszArgVBuffer += uiLength + 1;

		if( iIndex == 0 )
			m_uiCommandNameLength = uiLength;
	}

	return m_iArgc > 0;
}

CCommand::CCommand( const char* pszCommand )
{
	Initialize( pszCommand );
}

bool CCommand::Initialize( const char* pszCommand )
{
	assert( pszCommand );
	assert( *pszCommand );

	Reset();

	{
		const size_t uiLength = strlen( pszCommand );

		if( uiLength >= MAX_LENGTH )
			return false;

		strncpy( m_szCommandString, pszCommand, sizeof( m_szCommandString ) );
	}

	char szBuffer[ tokenization::MINIMUM_BUFFER_SIZE ];

	char* pszArgVBuffer = m_szArgsBuffer;

	while( 1 )
	{
		// skip whitespace up to a /n
		while( *pszCommand && ( *pszCommand ) <= ' ' && *pszCommand != '\n' )
			++pszCommand;

		if( *pszCommand == '\n' )
		{
			// a newline seperates commands in the buffer
			++pszCommand;
			break;
		}

		if( !( *pszCommand ) )
			break;

		pszCommand = tokenization::Parse( pszCommand, szBuffer, sizeof( szBuffer ) );
		if( !pszCommand ) break;

		const size_t uiLength = strlen( szBuffer );

		if( m_iArgc < MAX_TOKENS )
		{
			if( uiLength < static_cast<size_t>( MAX_LENGTH - ( pszArgVBuffer - m_szArgsBuffer ) ) )
			{
				m_pArgV[ m_iArgc ] = pszArgVBuffer;

				strncpy( pszArgVBuffer, szBuffer, uiLength + 1 );

				//Don't forget the null terminator for each argument
				pszArgVBuffer += uiLength + 1;

				++m_iArgc;
			}
			else
			{
				m_iArgc = 0;
				break;
			}
		}

		if( m_iArgc == 1 )
			m_uiCommandNameLength = uiLength;
	}

	return m_iArgc > 0;
}

CCommand::CCommand( const char* pszCommand, const char* pszValue )
{
	assert( pszCommand );
	assert( *pszCommand );
	assert( pszValue );

	Reset();

	const char* pArgV[ 2 ] = { pszCommand, pszValue };

	Initialize( 2, &pArgV, GetArg_ArgV );
}

CCommand::CCommand( const CCommand& other )
{
	*this = other;
}

bool CCommand::Initialize( const CCommand& other )
{
	if( this != &other )
	{
		Reset();

		return Initialize( other.ArgC(), const_cast<CCommand*>( &other ), &GetArg_CCommand );
	}

	return true;
}

CCommand& CCommand::operator=( const CCommand& other )
{
	Initialize( other );

	return *this;
}

void CCommand::Reset()
{
	m_iArgc = 0;
	m_uiCommandNameLength = 0;
	m_szCommandString[ 0 ] = '\0';
}

const char* CCommand::GetCommandString() const
{
	return m_iArgc ? m_szCommandString : "";
}

const char* CCommand::GetArgumentsString() const
{
	return m_uiCommandNameLength ? &m_szCommandString[ m_uiCommandNameLength ] : "";
}

const char* const* CCommand::ArgV() const
{
	return m_pArgV;
}

const char* CCommand::operator[]( const int iIndex ) const
{
	return Arg( iIndex );
}

const char* CCommand::Arg( const int iIndex ) const
{
	if( iIndex < 0 || iIndex >= ArgC() )
	{
		assert( !"CCommand::Arg: Index out of range!" );

		return "";
	}

	return m_pArgV[ iIndex ];
}

const char* CCommand::FindArg( const char* pszArgument ) const
{
	assert( pszArgument );
	assert( *pszArgument );

	for( int iIndex = 0; iIndex < m_iArgc; ++iIndex )
	{
		if( strcmp( pszArgument, Arg( iIndex ) ) == 0 )
		{
			return ( iIndex + 1 ) < m_iArgc ? Arg( iIndex + 1 ) : "";
		}
	}

	return nullptr;
}

int CCommand::FindIntArg( const char* pszArgument, const int iDefault ) const
{
	const char* pszValue = FindArg( pszArgument );

	if( !pszValue )
		return iDefault;

	return atoi( pszValue );
}
}