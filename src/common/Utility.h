#ifndef COMMON_UTILITY_H
#define COMMON_UTILITY_H

/**
*	Utility functionality.
*/

/**
*	Checks if a printf operation was successful
*/
inline bool PrintfSuccess( const int iRet, const size_t uiBufferSize )
{
	return iRet >= 0 && static_cast<size_t>( iRet ) < uiBufferSize;
}

#endif //COMMON_UTILITY_H