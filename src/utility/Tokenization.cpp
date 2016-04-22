#include <cassert>
#include <cctype>

#include "Tokenization.h"

namespace tokenization
{
bool IsControlChar( const char c )
{
	return 
		c == '{' || 
		c == '}' || 
		c == ')' || 
		c == '(' || 
		c == '\'' || 
		c == ',';
}

const char* Parse( const char* pszData, char* pszBuffer, const size_t uiBufferSize, bool* bBufferTooSmall )
{
	assert( pszBuffer != nullptr );
	assert( uiBufferSize >= 2 );	//Must be able to store at least one character + null terminator.

	char c;

	size_t len = 0;
	pszBuffer[ 0 ] = 0;

	if( bBufferTooSmall )
		*bBufferTooSmall = false;

	if( !pszData )
		return nullptr;

	// skip whitespace
skipwhite:
	while( ( c = *pszData ) <= ' ' )
	{
		if( c == '\0' )
			return nullptr;                    // end of file;
		++pszData;
	}

	// skip // comments
	if( c == '/' && pszData[ 1 ] == '/' )
	{
		while( *pszData && *pszData != '\n' )
			++pszData;
		goto skipwhite;
	}


	// handle quoted strings specially
	if( c == '\"' )
	{
		++pszData;
		while( 1 )
		{
			//Buffer too small.
			if( len >= uiBufferSize )
			{
				pszBuffer[ uiBufferSize - 1 ] = '\0';

				if( bBufferTooSmall )
					*bBufferTooSmall = true;

				return nullptr;
			}

			c = *pszData++;
			if( c == '\"' || !c )
			{
				pszBuffer[ len ] = '\0';
				return pszData;
			}
			pszBuffer[ len ] = c;
			++len;
		}
	}

	// parse single characters
	if( IsControlChar( c ) )
	{
		pszBuffer[ len ] = c;
		++len;
		pszBuffer[ len ] = '\0';
		return pszData + 1;
	}

	// parse a regular word
	do
	{
		//Buffer too small.
		if( len >= uiBufferSize )
		{
			pszBuffer[ uiBufferSize - 1 ] = '\0';

			if( bBufferTooSmall )
				*bBufferTooSmall = true;

			return nullptr;
		}

		pszBuffer[ len ] = c;
		++pszData;
		++len;
		c = *pszData;
		if( IsControlChar( c ) )
			break;
	}
	while( c > ' ' );

	pszBuffer[ len ] = '\0';
	return pszData;
}

bool TokenWaiting( const char* pszLine )
{
	const char* p = pszLine;

	while( *p && *p != '\n' )
	{
		if( !isspace( *p ) || isalnum( *p ) )
			return true;

		++p;
	}

	return false;
}
}