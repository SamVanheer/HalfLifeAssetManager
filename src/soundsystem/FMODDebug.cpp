#include "shared/Logging.h"

#include "FMODDebug.h"

#define FMOD_ERROR_MSG "FMOD Error: %s (error code: %d)"
#define FMOD_LINE_DATA "(File %s, Line %d)"

namespace soundsystem
{
bool _CheckFMODResult( const FMOD_RESULT result, const char* const pszFile, const int iLine )
{
	if( result == FMOD_OK )
		return false;

	const char* pszError = FMOD_ErrorString( result );

	if( pszFile )
	{
		Error( FMOD_ERROR_MSG FMOD_LINE_DATA "\n", static_cast<const char*>( pszError ), static_cast<int>( result ), pszFile, iLine );
	}
	else
	{
		Error( FMOD_ERROR_MSG "\n", static_cast<const char*>( pszError ), static_cast<int>( result ) );
	}

	return true;
}
}