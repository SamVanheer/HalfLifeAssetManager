#include "CKeyvalueNode.h"
#include "CKeyvalue.h"
#include "CKeyvalueBlock.h"

#include "CKeyvaluesParser.h"

namespace keyvalues
{
const char* CBaseKeyvaluesParser::ParseResultToString( const ParseResult result )
{
	switch( result )
	{
	case ParseResult::SUCCESS:			return "Success";
	case ParseResult::UNEXPECTED_EOB:	return "Unexpected End Of Buffer";
	case ParseResult::FORMAT_ERROR:		return "Format Error";
	case ParseResult::WRONG_NODE_TYPE:	return "Wrong Node Type";

	default:
	case ParseResult::UNKNOWN_ERROR:	return "Unknown Error";
	}
}

CBaseKeyvaluesParser::CBaseKeyvaluesParser( const CKeyvaluesParserSettings& settings, const bool fIsIterative )
	: m_Lexer( settings.lexerSettings )
	, m_iCurrentDepth( 0 )
	, m_Settings( settings )
	, m_fIsIterative( fIsIterative )
{
	Construct();
}

CBaseKeyvaluesParser::CBaseKeyvaluesParser( CKeyvaluesLexer::Memory_t& memory, const CKeyvaluesParserSettings& settings, const bool fIsIterative )
	: m_Lexer( memory, settings.lexerSettings )
	, m_iCurrentDepth( 0 )
	, m_Settings( settings )
	, m_fIsIterative( fIsIterative )
{
	Construct();
}

CBaseKeyvaluesParser::CBaseKeyvaluesParser( const char* const pszFilename, const CKeyvaluesParserSettings& settings, const bool fIsIterative )
	: m_Lexer( pszFilename, settings.lexerSettings )
	, m_iCurrentDepth( 0 )
	, m_Settings( settings )
	, m_fIsIterative( fIsIterative )
{
	Construct();
}

void CBaseKeyvaluesParser::Construct()
{
	//TODO: this sucks
	if( m_fIsIterative )
		++m_iCurrentDepth;
}

size_t CBaseKeyvaluesParser::GetReadOffset() const
{
	return m_Lexer.GetReadOffset();
}

void CBaseKeyvaluesParser::Initialize( CKeyvaluesLexer::Memory_t& memory )
{
	//This object will clean up the old state when it destructs
	CKeyvaluesLexer cleanup( memory, m_Settings.lexerSettings );

	m_Lexer.Swap( cleanup );

	m_iCurrentDepth = m_fIsIterative ? 1 : 0;
}

CBaseKeyvaluesParser::ParseResult CBaseKeyvaluesParser::ParseNext( CKeyvalueNode*& pNode, bool fParseFirst )
{
	ParseResult parseResult;

	CKeyvaluesLexer::ReadResult result = CKeyvaluesLexer::ReadResult::END_OF_BUFFER;

	if( fParseFirst )
	{
		result = m_Lexer.Read();
		parseResult = GetResultFor( result );

		if( parseResult != ParseResult::SUCCESS )
			return parseResult;
	}
	//ParseBlock already read the token, just check it

	bool fIsUnnamed = false;

	//The token we've parsed in must be a key, otherwise the format is incorrect
	if( m_Lexer.GetTokenType() != TokenType::KEY )
	{
		if( !m_Settings.lexerSettings.fAllowUnnamedBlocks )
			return ParseResult::FORMAT_ERROR;
		else
		{
			fIsUnnamed = true;
		}
	}

	CString szKey = !fIsUnnamed ? m_Lexer.GetToken() : "";

	//Only read again if named
	if( !fIsUnnamed )
	{
		result = m_Lexer.Read();

		//The lexer will validate the format for us and return FormatError if it failed
		if( ( parseResult = GetResultFor( result, result == CKeyvaluesLexer::ReadResult::READ_TOKEN ) ) != ParseResult::SUCCESS )
			return parseResult;
	}

	switch( m_Lexer.GetTokenType() )
	{
		//Parse in a block
	case TokenType::BLOCK_OPEN:
		{
			//If parsing the root, current depth is 1
			if( m_iCurrentDepth == 1 || m_Settings.fAllowNestedBlocks )
			{
				auto pBlock = new CKeyvalueBlock( szKey.CStr() );

				pNode = pBlock;

				parseResult = ParseBlock( pBlock, false );
			}
			else
			{
				//No nested blocks allowed; error out
				parseResult = ParseResult::FORMAT_ERROR;
			}
			break;
		}

	case TokenType::VALUE:
		{
			pNode = new CKeyvalue( szKey.CStr(), m_Lexer.GetToken().CStr() );
			parseResult = ParseResult::SUCCESS;
			break;
		}

		//Shouldn't be able to get here since the format is already checked, but just in case
	default: return ParseResult::FORMAT_ERROR;
	}

	return parseResult;
}

CBaseKeyvaluesParser::ParseResult CBaseKeyvaluesParser::ParseBlock( CKeyvalueBlock*& pBlock, bool fIsRoot )
{
	++m_iCurrentDepth;

	CKeyvalueBlock::Children_t children;

	CKeyvalueNode* pNode = nullptr;

	ParseResult parseResult;

	bool fContinue = true;

	do
	{
		const CKeyvaluesLexer::ReadResult result = m_Lexer.Read();

		if( result == CKeyvaluesLexer::ReadResult::END_OF_BUFFER && fIsRoot )
			--m_iCurrentDepth;

		parseResult = GetResultFor( result );

		if( parseResult != ParseResult::SUCCESS )
			fContinue = false;

		//End of this block
		if( m_Lexer.GetTokenType() == TokenType::BLOCK_CLOSE )
		{
			//Root blocks can't be closed by the buffer
			if( !fIsRoot )
			{
				--m_iCurrentDepth;
				pBlock->SetChildren( children );
			}
			else
				parseResult = ParseResult::FORMAT_ERROR;

			fContinue = false;
		}
		else if( m_Lexer.GetTokenType() == TokenType::NONE )
		{
			//End of the file while in a block
			if( !fIsRoot )
				parseResult = ParseResult::FORMAT_ERROR;
			else
				pBlock->SetChildren( children );

			fContinue = false;
		}
		else
		{
			//New keyvalue or block
			parseResult = ParseNext( pNode, false );

			if( parseResult == ParseResult::SUCCESS )
			{
				children.push_back( pNode );
			}
			else
			{
				delete pNode;
				fContinue = false;
			}

			pNode = nullptr;
		}
	}
	while( fContinue );

	if( parseResult != ParseResult::SUCCESS )
	{
		//Destroy all children to prevent leaks.
		for( auto pChild : children )
			delete pChild;
	}

	return parseResult;
}

CBaseKeyvaluesParser::ParseResult CBaseKeyvaluesParser::GetResultFor( const CKeyvaluesLexer::ReadResult result, bool fExpectedMore ) const
{
	switch( result )
	{
	case CKeyvaluesLexer::ReadResult::READ_TOKEN:		return ParseResult::SUCCESS;
	case CKeyvaluesLexer::ReadResult::END_OF_BUFFER:	return m_iCurrentDepth > 1 || fExpectedMore ? ParseResult::UNEXPECTED_EOB : ParseResult::SUCCESS;
	case CKeyvaluesLexer::ReadResult::FORMAT_ERROR:		return ParseResult::FORMAT_ERROR;

	default: return ParseResult::UNKNOWN_ERROR;
	}
}

CKeyvaluesParser::CKeyvaluesParser( const CKeyvaluesParserSettings& settings )
	: BaseClass( settings, false )
{
}

CKeyvaluesParser::CKeyvaluesParser( CKeyvaluesLexer::Memory_t& memory, const CKeyvaluesParserSettings& settings )
	: BaseClass( memory, settings, false )
{
}

CKeyvaluesParser::CKeyvaluesParser( const char* const pszFilename, const CKeyvaluesParserSettings& settings )
	: BaseClass( pszFilename, settings, false )
{
}

CKeyvaluesParser::~CKeyvaluesParser()
{
	delete m_pKeyvalues;
}

CKeyvalueBlock* CKeyvaluesParser::ReleaseKeyvalues()
{
	if( !m_pKeyvalues )
		return nullptr;

	auto pKeyvalues = m_pKeyvalues;

	m_pKeyvalues = nullptr;

	return pKeyvalues;
}

void CKeyvaluesParser::Initialize( CKeyvaluesLexer::Memory_t& memory )
{
	CBaseKeyvaluesParser::Initialize( memory );

	if( m_pKeyvalues )
	{
		delete m_pKeyvalues;
		m_pKeyvalues = nullptr;
	}
}

CKeyvaluesParser::ParseResult CKeyvaluesParser::Parse()
{
	if( m_pKeyvalues )
	{
		delete m_pKeyvalues;
		m_pKeyvalues = nullptr;
	}

	auto pRootNode = new CKeyvalueBlock( "" );

	ParseResult result = ParseBlock( pRootNode, true );

	if( result == ParseResult::SUCCESS )
	{
		m_pKeyvalues = pRootNode;
	}
	else
	{
		delete pRootNode;
	}

	return result;
}

CIterativeKeyvaluesParser::CIterativeKeyvaluesParser( const CKeyvaluesParserSettings& settings )
	: BaseClass( settings, true )
{
}

CIterativeKeyvaluesParser::CIterativeKeyvaluesParser( CKeyvaluesLexer::Memory_t& memory, const CKeyvaluesParserSettings& settings )
	: BaseClass( memory, settings, true )
{
}

CIterativeKeyvaluesParser::CIterativeKeyvaluesParser( const char* const pszFilename, const CKeyvaluesParserSettings& settings )
	: BaseClass( pszFilename, settings, true )
{
}

CIterativeKeyvaluesParser::ParseResult CIterativeKeyvaluesParser::ParseBlock( CKeyvalueBlock*& pBblock )
{
	pBblock = nullptr;

	CKeyvalueNode* pNode;

	ParseResult result = ParseNext( pNode, true );

	if( result != ParseResult::SUCCESS )
		return result;

	if( pNode->GetType() != NodeType::BLOCK )
	{
		delete pNode;
		return ParseResult::WRONG_NODE_TYPE;
	}

	pBblock = static_cast<CKeyvalueBlock*>( pNode );

	return ParseResult::SUCCESS;
}
}