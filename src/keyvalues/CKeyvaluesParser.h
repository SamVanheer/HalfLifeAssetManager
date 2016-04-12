#ifndef CKEYVALUESPARSER_H
#define CKEYVALUESPARSER_H

#include <memory>

#include "CKeyvaluesLexer.h"

class CKeyvalueNode;
class CKvBlockNode;
class CKeyvalues;

struct CKeyvaluesParserSettings
{
	CKeyvaluesLexerSettings lexerSettings;

	bool fAllowNestedBlocks;	//Keyvalues like entity data don't allow this

	CKeyvaluesParserSettings()
		: fAllowNestedBlocks( true )
	{
	}
};

/*
* Can parse in keyvalues text data and transform it into hierarchical data structures
* Internally uses CKeyvaluesLexer to tokenize the buffer's data
*/
class CBaseKeyvaluesParser
{
public:
	enum ParseResult
	{
		Success,
		EndOfData,	//All data parsed in
		UnexpectedEOB,
		FormatError,
		UnknownError,
		WrongNodeType
	};

	static const char* ParseResultToString( const ParseResult result );

public:
	const CKeyvaluesParserSettings& GetSettings() const { return m_Settings; }

	bool HasInputData() const { return m_Lexer.HasInputData(); }

	size_t GetReadOffset() const;

	/*
	* Initialize or reinitialize the memory buffer
	*/
	void Initialize( CKeyvaluesLexer::Memory_t& memory );

protected:
	/*
	* Construct an empty parser
	* An empty parser will return empty, but valid data structures
	*/
	CBaseKeyvaluesParser( const CKeyvaluesParserSettings& settings, const bool fIsIterative );

	/*
	* Construct a parser that will parse from the given memory buffer
	*/
	CBaseKeyvaluesParser( CKeyvaluesLexer::Memory_t& memory, const CKeyvaluesParserSettings& settings, const bool fIsIterative );

	/*
	* Construct a parser that will parse the given file
	*/
	CBaseKeyvaluesParser( const char* pszFileName, const CKeyvaluesParserSettings& settings, const bool fIsIterative );

	ParseResult ParseNext( std::shared_ptr<CKeyvalueNode>& node, bool fParseFirst );

	ParseResult ParseBlock( std::shared_ptr<CKvBlockNode>& block, bool fIsRoot );

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
	CBaseKeyvaluesParser( const CBaseKeyvaluesParser& );
	CBaseKeyvaluesParser& operator=( const CBaseKeyvaluesParser& );
};

/*
* Can parse in an entire keyvalues file at once
*/
class CKeyvaluesParser : public CBaseKeyvaluesParser
{
public:
	typedef CBaseKeyvaluesParser BaseClass;

public:
	CKeyvaluesParser( const CKeyvaluesParserSettings& settings = CKeyvaluesParserSettings() );

	CKeyvaluesParser( CKeyvaluesLexer::Memory_t& memory, const CKeyvaluesParserSettings& settings = CKeyvaluesParserSettings() );

	CKeyvaluesParser( const char* pszFileName, const CKeyvaluesParserSettings& settings = CKeyvaluesParserSettings() );

	std::shared_ptr<const CKeyvalues> GetKeyvalues() const { return m_Keyvalues; }
	std::shared_ptr<CKeyvalues> GetKeyvalues() { return m_Keyvalues; }

	void Initialize( CKeyvaluesLexer::Memory_t& memory );

	/*
	* Parse in the entire buffer
	* If successful, returns ParseResult::Success
	* If the end of the buffer is reached and all data was correctly parsed in, ParseResult::EndOfData is converted into ParseResult::Success
	* If the buffer ended before a fully formatted keyvalues format was parsed, returns ParseResult::UnexpectedEOB
	* If non-keyvalues data is found, returns ParseResult::FormatError
	* If an unknown lexer result occurs, returns ParseResult::UnknownError
	*/
	ParseResult Parse();

private:
	std::shared_ptr<CKeyvalues> m_Keyvalues;
};

/*
* Can parse in a keyvalues file one block at a time
*/
class CIterativeKeyvaluesParser : public CBaseKeyvaluesParser
{
public:
	typedef CBaseKeyvaluesParser BaseClass;

public:
	CIterativeKeyvaluesParser( const CKeyvaluesParserSettings& settings = CKeyvaluesParserSettings() );

	CIterativeKeyvaluesParser( CKeyvaluesLexer::Memory_t& memory, const CKeyvaluesParserSettings& settings = CKeyvaluesParserSettings() );

	/*
	* Parses a single block from the file
	* Use either this or Parse, do not use both at the same time, or you will get inaccurate results
	*/
	ParseResult ParseBlock( std::shared_ptr<CKvBlockNode>& block );
};

#endif //CKEYVALUESPARSER_H
