#ifndef CKEYVALUESLEXER_H
#define CKEYVALUESLEXER_H

#include "utility/CEscapeSequences.h"
#include "utility/CMemory.h"
#include "utility/CString.h"

#include "KeyvaluesConstants.h"

namespace keyvalues
{
/**
*	A lexer that can read in keyvalues text data and tokenize it
*/
class CKeyvaluesLexer
{
public:
	/**
	*	The result of a read operation.
	*/
	enum class ReadResult
	{
		READ_TOKEN,
		END_OF_BUFFER,
		FORMAT_ERROR
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
	*	Constructs an empty lexer
	*	@param escapeSeqConversion Escape sequences conversion rules.
	*	@param settings Lexer settings.
	*/
	CKeyvaluesLexer( CEscapeSequences& escapeSeqConversion, const CKeyvaluesLexerSettings& settings = CKeyvaluesLexerSettings() );

	/**
	*	Constructs a lexer that will read from the given memory
	*	The given memory will be empty after this constructor returns
	*	Expects a text buffer with all newlines normalized to \n
	*	@param memory Memory to use. The original memory buffer is no longer valid after this constructor returns.
	*	@param settings Lexer settings.
	*/
	CKeyvaluesLexer( Memory_t& memory, const CKeyvaluesLexerSettings& settings = CKeyvaluesLexerSettings() );

	/**
	*	Constructs a lexer that will read from the given memory
	*	The given memory will be empty after this constructor returns
	*	Expects a text buffer with all newlines normalized to \n
	*	@param memory Memory to use. The original memory buffer is no longer valid after this constructor returns.
	*	@param escapeSeqConversion Escape sequences conversion rules.
	*	@param settings Lexer settings.
	*/
	CKeyvaluesLexer( Memory_t& memory, CEscapeSequences& escapeSeqConversion, const CKeyvaluesLexerSettings& settings = CKeyvaluesLexerSettings() );

	/**
	*	Constructs a lexer that will read from the given file
	*	@param pszFilename Name of the file to read from. Must be non-null.
	*	@param settings Lexer settings.
	*/
	CKeyvaluesLexer( const char* const pszFilename, const CKeyvaluesLexerSettings& settings = CKeyvaluesLexerSettings() );

	/**
	*	Constructs a lexer that will read from the given file
	*	@param pszFilename Name of the file to read from. Must be non-null.
	*	@param escapeSeqConversion Escape sequences conversion rules.
	*	@param settings Lexer settings.
	*/
	CKeyvaluesLexer( const char* const pszFilename, CEscapeSequences& escapeSeqConversion, const CKeyvaluesLexerSettings& settings = CKeyvaluesLexerSettings() );

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
	TokenType GetTokenType() const { return m_TokenType; }

	/**
	*	Gets the current token.
	*/
	const CString& GetToken() const { return m_szToken; }

	/**
	*	Gets the escape sequences conversion object.
	*/
	CEscapeSequences* GetEscapeSeqConversion() const { return m_pEscapeSeqConversion; }

	/**
	*	Sets the escape sequences conversion object.
	*/
	void SetEscapeSeqConversion( CEscapeSequences& escapeSeqConversion ) { m_pEscapeSeqConversion = &escapeSeqConversion; }

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

	TokenType			m_TokenType;			//Type of the last token we read
	CString				m_szToken;				//The last token we read

	CEscapeSequences* m_pEscapeSeqConversion = &GetNoEscapeSeqConversion();

	CKeyvaluesLexerSettings m_Settings;

private:
	CKeyvaluesLexer( const CKeyvaluesLexer& ) = delete;
	CKeyvaluesLexer& operator=( const CKeyvaluesLexer& ) = delete;
};
}

#endif //CKEYVALUESLEXER_H