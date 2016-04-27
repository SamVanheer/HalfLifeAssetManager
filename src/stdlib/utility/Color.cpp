#include <cstdio>

#include "Color.h"

bool ParseColor( const char* pszValue, Color& color, const bool bParseAlpha )
{
	if( !pszValue || !( *pszValue ) )
		return false;

	unsigned int red, green, blue, alpha = color.GetAlpha();

	int iResult;
	
	if( bParseAlpha )
	{
		iResult = sscanf( pszValue, "%u %u %u %u", &red, &green, &blue, &alpha );
	}
	else
	{
		iResult = sscanf( pszValue, "%u %u %u", &red, &green, &blue );
	}

	if( iResult == EOF )
		return false;

	if( iResult != ( bParseAlpha ? 4 : 3 ) )
		return false;

	color.Set( red, green, blue, alpha );

	return true;
}

bool ColorToString( const Color& color, char* pszBuffer, size_t uiBufferSize, const bool bAddAlpha )
{
	if( !pszBuffer || !uiBufferSize )
		return false;

	int iResult;
	
	if( bAddAlpha )
	{
		iResult = snprintf( pszBuffer, uiBufferSize, "%u %u %u %u", color.GetRed(), color.GetGreen(), color.GetBlue(), color.GetAlpha() );
	}
	else
	{
		iResult = snprintf( pszBuffer, uiBufferSize, "%u %u %u", color.GetRed(), color.GetGreen(), color.GetBlue() );
	}

	if( iResult < 0 || static_cast<size_t>( iResult ) >= uiBufferSize )
		return false;

	return true;
}