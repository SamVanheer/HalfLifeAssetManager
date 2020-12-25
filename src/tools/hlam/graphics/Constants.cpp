#include <cstring>

#include "graphics/Constants.hpp"

const char* RenderModeToString( const RenderMode renderMode )
{
	switch( renderMode )
	{
	case RenderMode::WIREFRAME:			return "Wireframe";
	case RenderMode::FLAT_SHADED:		return "Flat Shaded";
	case RenderMode::SMOOTH_SHADED:		return "Smooth Shaded";
	case RenderMode::TEXTURE_SHADED:	return "Texture Shaded";

	default:							return "Invalid";
	}
}

RenderMode RenderModeFromString( const char* const pszString )
{
	if( !pszString || !( *pszString ) )
		return RenderMode::INVALID;

	for( int iRenderMode = static_cast<int>( RenderMode::FIRST ); iRenderMode < static_cast<int>( RenderMode::COUNT ); ++iRenderMode )
	{
		if( strcmp( RenderModeToString( static_cast<RenderMode>( iRenderMode ) ), pszString ) == 0 )
			return static_cast<RenderMode>( iRenderMode );
	}

	return RenderMode::INVALID;
}