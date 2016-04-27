#ifndef CKEYVALUESPARSER_H
#define CKEYVALUESPARSER_H

#include "CKeyvaluesLexer.h"

namespace keyvalues
{
class CKeyvalueNode;
class CKeyvalueBlock;

/**
*	Parser settings.
*/
struct CKeyvaluesParserSettings final
{
	CKeyvaluesLexerSettings lexerSettings;

	bool fAllowNestedBlocks;	//Keyvalues like entity data don't allow this

	CKeyvaluesParserSettings()
		: fAllowNestedBlocks( true )
	{
	}
};

/**
*	Can parse in keyvalues text data and transform it into hierarchical data structures
*	Internally uses CKeyvaluesLexer to tokenize the buffer's data
*/
class CBaseKeyvaluesParser
{
public:
	/**
	*	Parse result codes.
	*/
	enum class ParseResult
	{
		SUCCESS,
		UNEXPECTED_EOB,
		FORMAT_ERROR,
		UNKNOWN_ERROR,
		WRONG_NODE_TYPE
	};

	static const char* ParseResultToString( const ParseResult result );

public:
	/**
	*	Gets the parser settings.
	*/
	const CKeyvaluesParserSettings& GetSettings() const { return m_Settings; }

	/**
	*	Returns whether the parser has any input data.
	*/
	bool HasInputData() const { return m_Lexer.HasInputData(); }

	/**
	*	Gets the current read offset.
	*/
	size_t GetReadOffset() const;

	/**
	*	Initialize or reinitialize the memory buffer.
	*/
	void Initialize( CKeyvaluesLexer::Memory_t& memory );

	/**
	*	Gets the escape sequences conversion object.
	*/
	CEscapeSequences* GetEscapeSeqConversion() const { return m_Lexer.GetEscapeSeqConversion(); }

	/**
	*	Sets the escape sequences conversion object.
	*/
	void SetEscapeSeqConversion( CEscapeSequences& escapeSeqConversion ) { m_Lexer.SetEscapeSeqConversion( escapeSeqConversion ); }

protected:
	/**
	*	Construct an empty parser
	*	An empty parser will return empty, but valid data structures
	*/
	CBaseKeyvaluesParser( const CKeyvaluesParserSettings& settings, const bool fIsIterative );

	/**
	*	Construct a parser that will parse from the given memory buffer
	*/
	CBaseKeyvaluesParser( CKeyvaluesLexer::Memory_t& memory, const CKeyvaluesParserSettings& settings, const bool fIsIterative );

	/**
	*	Construct a parser that will parse the given file
	*/
	CBaseKeyvaluesParser( const char* const pszFilename, const CKeyvaluesParserSettings& settings, const bool fIsIterative );

	ParseResult ParseNext( CKeyvalueNode*& pNode, bool fParseFirst );

	ParseResult ParseBlock( CKeyvalueBlock*& pBlock, bool fIsRoot );

private:
	void Construct();

	ParseResult GetResultFor( const CKeyvaluesLexer::ReadResult result, bool fExpectedMore = false ) const;

private:
	CKeyvaluesLexer m_Lexer;

	/*
	* How deep we are in the parsing process.
	* If a keyvalue exists in the global scope, we're 1 level deep.
	* If we're inside a block that has no parent, we're 2 levels deep.
	*/
	int m_iCurrentDepth;

	CKeyvaluesParserSettings m_Settings;

	const bool m_fIsIterative;	//Required to make sure the current depth setting is valid for iterative calls

private:
	CBaseKeyvaluesParser( const CBaseKeyvaluesParser& ) = delete;
	CBaseKeyvaluesParser& operator=( const CBaseKeyvaluesParser& ) = delete;
};

/**
*	Parser that can parse in an entire keyvalues file at once
*/
class CKeyvaluesParser final : public CBaseKeyvaluesParser
{
public:
	typedef CBaseKeyvaluesParser BaseClass;

public:
	/**
	*	Constructs an empty parser with the given settings.
	*	@param settings Parser settings.
	*/
	CKeyvaluesParser( const CKeyvaluesParserSettings& settings = CKeyvaluesParserSettings() );

	/**
	*	Constructs a parser that reads from the given memory, and that has the given settings.
	*	@param memory Memory to read from.
	*	@param settings Parser settings.
	*/
	CKeyvaluesParser( CKeyvaluesLexer::Memory_t& memory, const CKeyvaluesParserSettings& settings = CKeyvaluesParserSettings() );

	/**
	*	Constructs a parser that reads from the given file, and that has the given settings.
	*	@param pszFilename Name of the file to read from.
	*	@param settings Parser settings.
	*/
	CKeyvaluesParser( const char* const pszFilename, const CKeyvaluesParserSettings& settings = CKeyvaluesParserSettings() );

	~CKeyvaluesParser();

	/**
	*	Gets the root keyvalues block.
	*/
	const CKeyvalueBlock* GetKeyvalues() const { return m_pKeyvalues; }

	/**
	*	@see GetKeyvalues() const
	*/
	CKeyvalueBlock* GetKeyvalues() { return m_pKeyvalues; }

	/**
	*	Releases ownership of the parser's keyvalues and returns them.
	*/
	CKeyvalueBlock* ReleaseKeyvalues();

	/**
	*	Initializes or reinitializes the parser with the given memory.
	*/
	void Initialize( CKeyvaluesLexer::Memory_t& memory );

	/**
	*	Parses in the entire buffer
	*	If successful, returns ParseResult::Success
	*	If the buffer ended before a fully formatted keyvalues format was parsed, returns ParseResult::UnexpectedEOB
	*	If non-keyvalues data is found, returns ParseResult::FormatError
	*	If an unknown lexer result occurs, returns ParseResult::UnknownError
	*/
	ParseResult Parse();

private:
	CKeyvalueBlock* m_pKeyvalues = nullptr;
};

/**
*	Parser that can parse in a keyvalues file one block at a time
*/
class CIterativeKeyvaluesParser final : public CBaseKeyvaluesParser
{
public:
	typedef CBaseKeyvaluesParser BaseClass;

public:
	/**
	*	Constructs an empty parser with the given settings.
	*	@param settings Parser settings.
	*/
	CIterativeKeyvaluesParser( const CKeyvaluesParserSettings& settings = CKeyvaluesParserSettings() );

	/**
	*	Constructs a parser that reads from the given memory, and that has the given settings.
	*	@param memory Memory to read from.
	*	@param settings Parser settings.
	*/
	CIterativeKeyvaluesParser( CKeyvaluesLexer::Memory_t& memory, const CKeyvaluesParserSettings& settings = CKeyvaluesParserSettings() );

	/**
	*	Constructs a parser that reads from the given file, and that has the given settings.
	*	@param pszFilename Name of the file to read from.
	*	@param settings Parser settings.
	*/
	CIterativeKeyvaluesParser( const char* const pszFilename, const CKeyvaluesParserSettings& settings = CKeyvaluesParserSettings() );

	/**
	*	Parses a single block from the file. You will have to free the block yourself when you're done with it.
	*/
	ParseResult ParseBlock( CKeyvalueBlock*& pBlock );
};
}

#endif //CKEYVALUESPARSER_H
