#include <cassert>
#include <utility>

#include "shared/Platform.h"

#ifdef WIN32
//Nothing so far
#else
#include <dlfcn.h>
#endif

#include "CLibrary.h"

CLibrary::CLibrary()
{
}

CLibrary::CLibrary( CLibrary&& other )
{
	std::swap( m_hLibrary, other.m_hLibrary );
}

CLibrary& CLibrary::operator=( CLibrary&& other )
{
	Free();

	std::swap( m_hLibrary, other.m_hLibrary );

	return *this;
}

CLibrary::~CLibrary()
{
	Free();
}

#ifdef WIN32
bool CLibrary::Load( const char* const pszFilename )
{
	assert( pszFilename );

	Free();

	m_hLibrary = LoadLibraryA( pszFilename );

	return IsLoaded();
}

void CLibrary::Free()
{
	if( IsLoaded() )
	{
		FreeLibrary( static_cast<HMODULE>( m_hLibrary ) );
		m_hLibrary = NULL_HANDLE();
	}
}

void* CLibrary::GetFunctionAddress( const char* const pszName ) const
{
	assert( IsLoaded() );
	assert( pszName );

	return GetProcAddress( static_cast<HMODULE>( m_hLibrary ), pszName );
}
#else
bool CLibrary::Load( const char* const pszFilename )
{
	assert( pszFilename );

	Free();

	m_hLibrary = dlopen( pszFilename, RTLD_NOW );

	return IsLoaded();
}

void CLibrary::Free()
{
	if( IsLoaded() )
	{
		dlclose( static_cast<void*>( m_hLibrary ) );
		m_hLibrary = NULL_HANDLE();
	}
}

void* CLibrary::GetFunctionAddress( const char* const pszName ) const
{
	assert( IsLoaded() );
	assert( pszName );

	return dlsym( static_cast<void*>( m_hLibrary ), pszName );
}
#endif