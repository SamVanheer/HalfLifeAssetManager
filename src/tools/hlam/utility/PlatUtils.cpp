#include <cassert>
#include <vector>

#include "core/shared/Platform.hpp"

#ifndef WIN32
#include <unistd.h>
#endif

#include "PlatUtils.hpp"

namespace plat
{
std::string GetExeFileName( bool* pSuccess )
{
	if( pSuccess )
		*pSuccess = false;

	std::vector<char> vecFileName;

	vecFileName.resize( MAX_PATH_LENGTH );

#ifdef WIN32
	size_t uiCount = 0;

	//Double the buffer size up to 10 times before quitting.
	do
	{
		const DWORD uiLength = GetModuleFileNameA( NULL, vecFileName.data(), vecFileName.size() );

		//Buffer too small.
		if( uiLength < vecFileName.size() )
		{
			if( pSuccess )
				*pSuccess = true;

			return std::string( vecFileName.begin(), vecFileName.end() );
		}

		const auto newSize = vecFileName.size() * 2;
		
		//Account for overflows.
		if( newSize < vecFileName.size() )
			return "";

		vecFileName.resize( newSize );
	}
	while( uiCount++ < 10 );

#else
	//TODO: untested.
	char szBuffer[ MAX_PATH ];

	ssize_t iResult = readlink( "/proc/self/exe", szBuffer, sizeof( szBuffer ) - 1 );

	if( iResult != -1 )
	{
		if( pSuccess )
			*pSuccess = true;

		szBuffer[ iResult ] = '\0';

		return szBuffer;
	}
#endif

	return "";
}
}