#ifndef CKEYVALUESLEXER_H
#define CKEYVALUESLEXER_H

#include "utility/CMemory.h"
#include "utility/CString.h"

#include "KeyvaluesConstants.h"

/**
*	A lexer that can read in keyvalues text data and tokenize it
*/
class CKeyvaluesLexer
{
public:
	/**
	*	The result of a read operation.
	*/
	enum ReadResult
	{
		ReadToken,
		EndOfBuffer,
		FormatError
	};

	typedef size_t size_type;

	typedef CMemory<size_type> Memory_t;

public:
	/**
	*	Constructs an empty lexer
	*	@param settings Lexer settings.
	*/
	CKeyvaluesLexer( const CKeyvaluesLexerSettings& settings = CKeyvaluesLexerSettings() );

	/**
	*	Constructs a lexer that will read from the given memory
	*	The given memory will be empty after this constructor returns
	*	Expects a text buffer with all newlines normalized to \n
	*	@param memory Memory to use. The original memory buffer is no longer valid after this constructor returns.
	*	@param settings Lexer settings.
	*/
	CKeyvaluesLexer( Memory_t& memory, const CKeyvaluesLexerSettings& settings = CKeyvaluesLexerSettings() );

	/**
	*	Constructs a lexer that will read from the given file
	*	@param pszFilename Name of the file to read from. Must be non-null.
	*	@param settings Lexer settings.
	*/
	CKeyvaluesLexer( const char* const pszFilename, const CKeyvaluesLexerSettings& settings = CKeyvaluesLexerSettings() );

	/**
	*	Returns whether the lexer has any input data.
	*/
	bool HasInputData() const;

	/**
	*	Gets the lexer's data.
	*/
	const Memory_t& GetMemory() const { return m_Memory; }

	/**
	*	Gets the current read offset.
	*/
	size_type GetReadOffset() const;

	/**
	*	Gets the current token type.
	*/
	KeyvalueTokenType GetTokenType() const { return m_TokenType; }

	/**
	*	Gets the current token.
	*/
	const CString& GetToken() const { return m_szToken; }

	/**
	*	Gets the lexer's settings.
	*/
	const CKeyvaluesLexerSettings& GetSettings() const { return m_Settings; }

	/**
	*	Resets the read position to the beginning of the input data.
	*/
	void Reset();

	/**
	*	Swaps lexer data.
	*/
	void Swap( CKeyvaluesLexer& other );

	/**
	*	Reads as many characters as needed to produce the next token
	*	Returns EndOfBuffer if no more tokens were found
	*/
	ReadResult Read();

private:
	bool IsValidReadPosition();

	void SkipWhitespace();

	/**
	*	Returns true if comments were skipped
	*/
	bool SkipComments();
	bool SkipCommentLine();

	/**
	*	Reads whatever is next. Handles quoted strings specially
	*	Advances the current position pointer
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
	CKeyvaluesLexer( const CKeyvaluesLexer& ) = delete;
	CKeyvaluesLexer& operator=( const CKeyvaluesLexer& ) = delete;
};

#endif //CKEYVALUESLEXER_H