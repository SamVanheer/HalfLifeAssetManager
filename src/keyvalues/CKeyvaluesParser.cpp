#include "CKeyvalueNode.h"
#include "CKeyvalue.h"
#include "CKvBlockNode.h"

#include "CKeyvaluesParser.h"

const char* CBaseKeyvaluesParser::ParseResultToString( const ParseResult result )
{
	switch( result )
	{
	case Success:			return "Success";
	case EndOfData:			return "EndOfData";
	case UnexpectedEOB:		return "UnexpectedEOB";
	case FormatError:		return "FormatError";
	case WrongNodeType:		return "WrongNodeType";

	default:
	case UnknownError:		return "UnknownError";
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

	CKeyvaluesLexer::ReadResult result = CKeyvaluesLexer::EndOfBuffer;

	if( fParseFirst )
	{
		result = m_Lexer.Read();
		parseResult = GetResultFor( result );

		if( parseResult != Success )
			return parseResult;
	}
	//ParseBlock already read the token, just check it

	bool fIsUnnamed = false;

	//The token we've parsed in must be a key, otherwise the format is incorrect
	if( m_Lexer.GetTokenType() != KVToken_Key )
	{
		if( !m_Settings.lexerSettings.fAllowUnnamedBlocks )
			return FormatError;
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
		if( ( parseResult = GetResultFor( result, result == CKeyvaluesLexer::ReadToken ) ) != Success )
			return parseResult;
	}

	switch( m_Lexer.GetTokenType() )
	{
		//Parse in a block
	case KVToken_BlockOpen:
		{
			//If parsing the root, current depth is 1
			if( m_iCurrentDepth == 1 || m_Settings.fAllowNestedBlocks )
			{
				CKvBlockNode* pBlock = new CKvBlockNode( szKey.CStr() );

				pNode = pBlock;

				parseResult = ParseBlock( pBlock, false );
			}
			else
			{
				//No nested blocks allowed; error out
				parseResult = FormatError;
			}
			break;
		}

	case KVToken_Value:
		{
			pNode = new CKeyvalue( szKey.CStr(), m_Lexer.GetToken().CStr() );
			parseResult = Success;
			break;
		}

		//Shouldn't be able to get here since the format is already checked, but just in case
	default: return FormatError;
	}

	return parseResult;
}

CBaseKeyvaluesParser::ParseResult CBaseKeyvaluesParser::ParseBlock( CKvBlockNode*& pBlock, bool fIsRoot )
{
	++m_iCurrentDepth;

	CKvBlockNode::Children_t children;

	CKeyvalueNode* pNode = nullptr;

	ParseResult parseResult;

	bool fContinue = true;

	do
	{
		const CKeyvaluesLexer::ReadResult result = m_Lexer.Read();

		if( result == CKeyvaluesLexer::EndOfBuffer && fIsRoot )
			--m_iCurrentDepth;

		parseResult = GetResultFor( result );

		if( parseResult != Success )
			fContinue = false;

		//End of this block
		if( m_Lexer.GetTokenType() == KVToken_BlockClose )
		{
			//Root blocks can't be closed by the buffer
			if( !fIsRoot )
			{
				--m_iCurrentDepth;
				pBlock->SetChildren( children );
			}
			else
				parseResult = FormatError;

			fContinue = false;
		}
		else if( m_Lexer.GetTokenType() == KVToken_None )
		{
			//End of the file while in a block
			if( !fIsRoot )
				parseResult = FormatError;
			else
				pBlock->SetChildren( children );

			fContinue = false;
		}
		else
		{
			//New keyvalue or block
			parseResult = ParseNext( pNode, false );

			if( parseResult == Success )
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

	return parseResult;
}

CBaseKeyvaluesParser::ParseResult CBaseKeyvaluesParser::GetResultFor( const CKeyvaluesLexer::ReadResult result, bool fExpectedMore ) const
{
	switch( result )
	{
	case CKeyvaluesLexer::ReadToken: return Success;
	case CKeyvaluesLexer::EndOfBuffer: return m_iCurrentDepth > 1 || fExpectedMore ? UnexpectedEOB : EndOfData;
	case CKeyvaluesLexer::FormatError: return FormatError;

	default: return UnknownError;
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

CKvBlockNode* CKeyvaluesParser::ReleaseKeyvalues()
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

	CKvBlockNode* pRootNode = new CKvBlockNode( "" );

	ParseResult result = ParseBlock( pRootNode, true );

	//Convert successful parse into Success
	if( result == EndOfData )
	{
		result = Success;
	}

	if( result == Success )
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

CIterativeKeyvaluesParser::ParseResult CIterativeKeyvaluesParser::ParseBlock( CKvBlockNode*& pBblock )
{
	pBblock = nullptr;

	CKeyvalueNode* pNode;

	ParseResult result = ParseNext( pNode, true );

	if( result != Success )
		return result;

	if( pNode->GetType() != KVNode_Block )
	{
		delete pNode;
		return WrongNodeType;
	}

	pBblock = static_cast<CKvBlockNode*>( pNode );

	return Success;
}