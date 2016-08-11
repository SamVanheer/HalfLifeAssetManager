#include <cassert>
#include <filesystem>
#include <vector>

#include "core/shared/Platform.h"

#ifndef WIN32
#include <unistd.h>
#endif

#include "PlatUtils.h"

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
	std::error_code error_code;

	auto path = std::experimental::filesystem::read_symlink( "/proc/self/exe", error_code );

	if( !error_code )
	{
		if( pSuccess )
			*pSuccess = true;

		return path.string();
	}
#endif

	return "";
}
}