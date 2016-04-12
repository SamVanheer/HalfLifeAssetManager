#include <cctype>
#include <cstring>
#include <cstdio>
#include <algorithm>

#include "common/Logging.h"

#include "CKeyvaluesLexer.h"

CKeyvaluesLexer::CKeyvaluesLexer( const CKeyvaluesLexerSettings& settings )
	: m_TokenType( KVToken_None )
	, m_pszCurrentPosition( nullptr )
	, m_Settings( settings )
{
}

CKeyvaluesLexer::CKeyvaluesLexer( Memory_t& memory, const CKeyvaluesLexerSettings& settings )
	: m_TokenType( KVToken_None )
	, m_Settings( settings )
{
	m_Memory.Swap( memory );

	m_pszCurrentPosition = reinterpret_cast<const char*>( m_Memory.GetMemory() );
}

CKeyvaluesLexer::CKeyvaluesLexer( const char* pszFileName, const CKeyvaluesLexerSettings& settings )
	: m_TokenType( KVToken_None )
	, m_pszCurrentPosition( nullptr )
	, m_Settings( settings )
{
	if( pszFileName && *pszFileName )
	{
		FILE* pFile = fopen( pszFileName, "rb" );

		if( pFile )
		{
			fseek( pFile, 0, SEEK_END );
			const int iSizeInBytes = ftell( pFile );
			fseek( pFile, 0, SEEK_SET );

			CKeyvaluesLexer::Memory_t memory( iSizeInBytes );

			const int iRead = fread( memory.GetMemory(), 1, iSizeInBytes, pFile );

			fclose( pFile );

			if( iRead == iSizeInBytes )
			{
				m_Memory.Swap( memory );

				m_pszCurrentPosition = reinterpret_cast<const char*>( m_Memory.GetMemory() );
			}
		}
	}
}

bool CKeyvaluesLexer::HasInputData() const
{
	return m_Memory.HasMemory();
}

CKeyvaluesLexer::size_type CKeyvaluesLexer::GetReadOffset() const
{
	return m_pszCurrentPosition ? m_pszCurrentPosition - reinterpret_cast<const char*>( m_Memory.GetMemory() ) : 0;
}

void CKeyvaluesLexer::Reset()
{
	m_pszCurrentPosition = reinterpret_cast<const char*>( m_Memory.GetMemory() );
	m_TokenType = KVToken_None;
	m_szToken = "";
}

void CKeyvaluesLexer::Swap( CKeyvaluesLexer& other )
{
	if( this != & other )
	{
		m_Memory.Swap( other.m_Memory );
		std::swap( m_pszCurrentPosition, other.m_pszCurrentPosition );
		std::swap( m_TokenType, other.m_TokenType );
		std::swap( m_szToken, other.m_szToken );
		std::swap( m_Settings, other.m_Settings );
	}
}

CKeyvaluesLexer::ReadResult CKeyvaluesLexer::Read()
{
	ReadResult result = ReadNextToken();

	//Reset last token to none to prevent invalid token types
	if( result == EndOfBuffer )
		m_TokenType = KVToken_None;

	return result;
}

bool CKeyvaluesLexer::IsValidReadPosition()
{
	const size_t offset = ( m_pszCurrentPosition - reinterpret_cast<const char*>( m_Memory.GetMemory() ) );
	return static_cast<size_type>( m_pszCurrentPosition - reinterpret_cast<const char*>( m_Memory.GetMemory() ) ) < m_Memory.GetSize();
}

void CKeyvaluesLexer::SkipWhitespace()
{
	while( IsValidReadPosition() && isspace( *m_pszCurrentPosition ) )
	{
		++m_pszCurrentPosition;
	}
}

bool CKeyvaluesLexer::SkipComments()
{
	bool fSkipped = false;

	if( ( fSkipped = SkipCommentLine() ) )
	{
		//Skip any more lines
		while( SkipCommentLine() )
		{
		}
	}

	return fSkipped;
}

bool CKeyvaluesLexer::SkipCommentLine()
{
	SkipWhitespace();

	if( !IsValidReadPosition() )
		return false;

	if( *m_pszCurrentPosition == '/' && *( m_pszCurrentPosition + 1 ) == '/' )
	{
		m_pszCurrentPosition += 2;

		//Skip all characters, including the newline
		while( *( m_pszCurrentPosition - 1 ) != '\n' && IsValidReadPosition() )
		{
			++m_pszCurrentPosition;
		}

		return true;
	}

	return false;
}

CKeyvaluesLexer::ReadResult CKeyvaluesLexer::ReadNext( const char*& pszBegin, const char*& pszEnd, bool& fWasQuoted )
{
	ReadResult result = EndOfBuffer;

	//Found a quoted string, parse in until we find the next quote or newline
	//TODO: parse escape sequences properly
	switch( *m_pszCurrentPosition )
	{
	case KeyvalueControl_Quote:
		{
			fWasQuoted = true;

			++m_pszCurrentPosition;

			pszBegin = m_pszCurrentPosition;

			while( *m_pszCurrentPosition != KeyvalueControl_Quote && *m_pszCurrentPosition != '\n' && IsValidReadPosition() )
				++m_pszCurrentPosition;

			pszEnd = m_pszCurrentPosition;

			if( IsValidReadPosition() )
			{
				result = ReadToken;

				//Advance past the closing quote or newline
				//TODO: track line number
				if( *m_pszCurrentPosition != KeyvalueControl_Quote && m_Settings.fLogWarnings )
					Error( "CKeyvaluesLexer::ReadNext: unclosed quote!\n" );

				++m_pszCurrentPosition;
			}

			break;
		}

		//Read open and close as single characters
	case KeyvalueControl_BlockOpen:
	case KeyvalueControl_BlockClose:
		{
			pszBegin = m_pszCurrentPosition;
			++m_pszCurrentPosition;
			pszEnd = m_pszCurrentPosition;

			result = ReadToken;

			break;
		}

	default:
		{
			fWasQuoted = false;

			//Found an unquoted value; read until first whitespace

			pszBegin = m_pszCurrentPosition;

			while( !isspace( *m_pszCurrentPosition ) && IsValidReadPosition() )
				++m_pszCurrentPosition;

			pszEnd = m_pszCurrentPosition;

			//Always consider this a successful read, in case this is the last token with no newline after it
			result = ReadToken;

			break;
		}
	}

	return result;
}

CKeyvaluesLexer::ReadResult CKeyvaluesLexer::ReadNextToken()
{
	//No buffer, or reached end
	if( !IsValidReadPosition() )
	{
		return EndOfBuffer;
	}

	SkipComments();

	//Nothing left to read
	if( !IsValidReadPosition() )
	{
		return EndOfBuffer;
	}

	//SkipComments places us at the first non-whitespace character that isn't a comment

	const char* pszBegin, * pszEnd;
	bool fWasQuoted;

	ReadResult result = ReadNext( pszBegin, pszEnd, fWasQuoted );

	if( result == ReadToken )
	{
		if( strncmp( pszBegin, "{", pszEnd - pszBegin ) == 0 )
		{
			//Can only open a block after a key
			if( m_TokenType != KVToken_Key && !m_Settings.fAllowUnnamedBlocks )
			{
				if( m_Settings.fLogErrors )
					Error( "CKeyvaluesLexer::ReadNextToken: illegal block open '%c'!\n", KeyvalueControl_BlockOpen );

				result = FormatError;
				m_szToken = "";
				m_TokenType = KVToken_None;
			}
			else
			{
				m_szToken = KeyvalueControl_BlockOpen;
				m_TokenType = KVToken_BlockOpen;
			}
		}
		else if( strncmp( pszBegin, "}", pszEnd - pszBegin ) == 0 )
		{
			//Can only close a block after a block open, close or value
			if( m_TokenType != KVToken_Value && m_TokenType != KVToken_BlockOpen && m_TokenType != KVToken_BlockClose )
			{
				if( m_Settings.fLogErrors )
					Error( "CKeyvaluesLexer::ReadNextToken: illegal block close '%c'!\n", KeyvalueControl_BlockClose );
				result = FormatError;
				m_szToken = "";
				m_TokenType = KVToken_None;
			}
			else
			{
				m_szToken = KeyvalueControl_BlockClose;
				m_TokenType = KVToken_BlockClose;
			}
		}
		else
		{
			m_szToken.Assign( pszBegin, 0, pszEnd - pszBegin );

			//If the previous token was a key, this becomes a value
			if( m_TokenType == KVToken_Key )
				m_TokenType = KVToken_Value;
			else
				m_TokenType = KVToken_Key;
		}
	}

	return result;
}