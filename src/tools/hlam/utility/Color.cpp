#include <cstdio>
#include <sstream>

#include "Color.hpp"

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

std::string ColorToString( const Color& color, const bool bAddAlpha )
{
	std::ostringstream stream;

	stream << std::to_string(color.GetRed()) << ' ' << std::to_string(color.GetGreen()) << ' ' << std::to_string(color.GetBlue());

	if (bAddAlpha)
	{
		stream << ' ' << std::to_string(color.GetAlpha());
	}
	
	return stream.str();
}