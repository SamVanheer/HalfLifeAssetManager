#include <cstring>

#include "graphics/GraphicsConstants.hpp"

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
