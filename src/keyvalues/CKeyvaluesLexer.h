#ifndef CKEYVALUESLEXER_H
#define CKEYVALUESLEXER_H

#include "utility/CMemory.h"
#include "utility/CString.h"

#include "KeyvaluesConstants.h"

/*
* A lexer that can read in keyvalues text data and tokenize it
*/
class CKeyvaluesLexer
{
public:
	enum ReadResult
	{
		ReadToken,
		EndOfBuffer,
		FormatError
	};

	typedef size_t size_type;

	typedef CMemory<size_type> Memory_t;

public:
	/*
	* Constructs an empty lexer
	*/
	CKeyvaluesLexer( const CKeyvaluesLexerSettings& settings = CKeyvaluesLexerSettings() );

	/*
	* Constructs a lexer that will read from the given memory
	* The given memory will be empty after this constructor returns
	* Expects a text buffer with all newlines normalized to \n
	*/
	CKeyvaluesLexer( Memory_t& memory, const CKeyvaluesLexerSettings& settings = CKeyvaluesLexerSettings() );

	/*
	* Constructs a lexer that will read from the given file
	*/
	CKeyvaluesLexer( const char* pszFileName, const CKeyvaluesLexerSettings& settings = CKeyvaluesLexerSettings() );

	bool HasInputData() const;

	const Memory_t& GetMemory() const { return m_Memory; }

	size_type GetReadOffset() const;

	KeyvalueTokenType GetTokenType() const { return m_TokenType; }

	const CString& GetToken() const { return m_szToken; }

	const CKeyvaluesLexerSettings& GetSettings() const { return m_Settings; }

	void Reset();

	void Swap( CKeyvaluesLexer& other );

	/*
	* Reads as many characters as needed to produce the next token
	* Returns EndOfBuffer if no more tokens were found
	*/
	ReadResult Read();

private:
	bool IsValidReadPosition();

	void SkipWhitespace();

	/*
	* Returns true if comments were skipped
	*/
	bool SkipComments();
	bool SkipCommentLine();

	/*
	* Reads whatever is next. Handles quoted strings specially
	* Advances the current position pointer
	*/
	ReadResult ReadNext( const char*& pszBegin, const char*& pszEnd, bool& fWasQuoted );

	ReadResult ReadNextToken();

private:
	Memory_t			m_Memory;
	const char*			m_pszCurrentPosition;

	KeyvalueTokenType	m_TokenType;			//Type of the last token we read
	CString				m_szToken;				//The last token we read

	CKeyvaluesLexerSettings m_Settings;

private:
	CKeyvaluesLexer( const CKeyvaluesLexer& );
	CKeyvaluesLexer& operator=( const CKeyvaluesLexer& );
};

#endif //CKEYVALUESLEXER_H