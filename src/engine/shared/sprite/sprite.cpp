#include "core/shared/Platform.h"

#include "sprite.h"

namespace sprite
{
const char* TypeToString( const Type_t type )
{
	switch( type )
	{
	case Type_t::VP_PARALLEL_UPRIGHT:	return "VP_PARALLEL_UPRIGHT";
	case Type_t::FACING_UPRIGHT:		return "FACING_UPRIGHT";
	case Type_t::VP_PARALLEL:			return "VP_PARALLEL";
	case Type_t::ORIENTED:				return "ORIENTED";
	case Type_t::VP_PARALLEL_ORIENTED:	return "VP_PARALLEL_ORIENTED";
	default:							return "UNKNOWN";
	}
}

Type_t StringToType( const char* const pszString, bool* pbSuccess )
{
	if( !pszString )
	{
		if( pbSuccess )
			*pbSuccess = false;

		return Type_t::VP_PARALLEL_UPRIGHT;
	}

	Type_t result;

	if( strcasecmp( pszString, "VP_PARALLEL_UPRIGHT" ) == 0 )
	{
		result = Type_t::VP_PARALLEL_UPRIGHT;
	}
	else if( strcasecmp( pszString, "FACING_UPRIGHT" ) == 0 )
	{
		result = Type_t::FACING_UPRIGHT;
	}
	else if( strcasecmp( pszString, "VP_PARALLEL" ) == 0 )
	{
		result = Type_t::VP_PARALLEL;
	}
	else if( strcasecmp( pszString, "ORIENTED" ) == 0 )
	{
		result = Type_t::ORIENTED;
	}
	else if( strcasecmp( pszString, "VP_PARALLEL_ORIENTED" ) == 0 )
	{
		result = Type_t::VP_PARALLEL_ORIENTED;
	}
	else
	{
		if( pbSuccess )
			*pbSuccess = false;

		return Type_t::VP_PARALLEL_UPRIGHT;
	}

	if( pbSuccess )
		*pbSuccess = true;

	return result;
}

const char* TexFormatToString( const TexFormat::TexFormat format )
{
	switch( format )
	{
	case TexFormat::SPR_NORMAL:			return "NORMAL";
	case TexFormat::SPR_ADDITIVE:		return "ADDITIVE";
	case TexFormat::SPR_INDEXALPHA:		return "INDEXALPHA";
	case TexFormat::SPR_ALPHTEST:		return "ALPHATEST";
	default:							return "UNKNOWN";
	}
}

TexFormat::TexFormat StringToTexFormat( const char* const pszString, bool* pbSuccess )
{
	if( !pszString )
	{
		if( pbSuccess )
			*pbSuccess = false;

		return TexFormat::SPR_NORMAL;
	}

	TexFormat::TexFormat result;

	if( strcasecmp( pszString, "NORMAL" ) == 0 )
	{
		result = TexFormat::SPR_NORMAL;
	}
	else if( strcasecmp( pszString, "ADDITIVE" ) == 0 )
	{
		result = TexFormat::SPR_ADDITIVE;
	}
	else if( strcasecmp( pszString, "INDEXALPHA" ) == 0 )
	{
		result = TexFormat::SPR_INDEXALPHA;
	}
	else if( strcasecmp( pszString, "ALPHATEST" ) == 0 )
	{
		result = TexFormat::SPR_ALPHTEST;
	}
	else
	{
		if( pbSuccess )
			*pbSuccess = false;

		return TexFormat::SPR_NORMAL;
	}

	if( pbSuccess )
		*pbSuccess = true;

	return result;
}
}