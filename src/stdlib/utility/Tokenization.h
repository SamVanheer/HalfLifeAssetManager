#ifndef UTILITY_TOKENIZATION_H
#define UTILITY_TOKENIZATION_H

/**
*	@defgroup Tokenization Tokenization utility code.
*	@{
*/
namespace tokenization
{
/**
*	Recommended minimum buffer size for tokenization.
*/
const size_t MINIMUM_BUFFER_SIZE = 1500;

/**
*	Returns whether the character c is a control character.
*	Control characters are parsed on their own, and delimit tokens.
*/
bool IsControlChar( const char c );

/**
*	Parses a token out of a string.
*	@param pszData string to parse.
*	@param pszBuffer Pointer to a buffer that will store the token. Must be able to store at least 2 characters or more.
*	@param uiBufferSize Size of pszBuffer, in characters.
*	@param bBufferTooSmall Optional parameter that can be used to detect buffer too small problems.
*	@return If a token was parsed, returns the position of the next token in pszData. 
*			If EOF was encountered before a token was found, returns null.
*			If the buffer is too small, returns null and sets bBufferTooSmall to true.
*/
const char* Parse( const char* pszData, char* pszBuffer, const size_t uiBufferSize, bool* bBufferTooSmall = nullptr );

/**
*	Returns true if additional data is waiting to be processed on this line.
*	@param pszLine Line to check.
*	@return true if there is data on this line, false otherwise.
*/
bool TokenWaiting( const char* pszLine );
}
/**@}*/

#endif //UTILITY_TOKENIZATION_H
