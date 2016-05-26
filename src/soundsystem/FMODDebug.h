#ifndef SOUNDSYSTEM_FMODDEBUG_H
#define SOUNDSYSTEM_FMODDEBUG_H

#include "fmod_errors.h"

/**
*	@ingroup SoundSystem
*
*	@{
*/

namespace soundsystem
{
/**
*	Checks the result of an FMOD operation. If the result is not ok, returns true.
*	@param result Result to check.
*	@param pszFile Filename where the check is occurring in.
*	@param iLine Line number.
*	@return true if the result is not ok.
*/
bool _CheckFMODResult( const FMOD_RESULT result, const char* const pszFile, const int iLine );
}

#ifdef _DEBUG
/**
*	Used to check FMOD result values. If the value is not FMOD_OK, logs an error and returns true.
*	In debug builds, this will output the file and line number as well.
*	@see soundsystem::_CheckFMODResult
*/
#define CheckFMODResult( result ) ::soundsystem::_CheckFMODResult( result, __FILE__, __LINE__ )
#else
#define CheckFMODResult( result ) ::soundsystem::_CheckFMODResult( result, nullptr, -1 )
#endif

/** @} */

#endif //SOUNDSYSTEM_FMODDEBUG_H