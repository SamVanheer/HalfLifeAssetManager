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
	std::swap( m_szName, other.m_szName );
	std::swap( m_hLibrary, other.m_hLibrary );
}

CLibrary& CLibrary::operator=( CLibrary&& other )
{
	Free();

	std::swap( m_szName, other.m_szName );
	std::swap( m_hLibrary, other.m_hLibrary );

	return *this;
}

CLibrary::~CLibrary()
{
	Free();
}

bool CLibrary::Load( const char* const pszFilename )
{
	assert( pszFilename );

	Free();

	m_hLibrary = DoLoad( pszFilename );

	if( !IsLoaded() )
		return false;

	m_szName = pszFilename;

	return true;
}

void CLibrary::Free()
{
	if( IsLoaded() )
	{
		DoFree( m_hLibrary );
		m_hLibrary = NULL_HANDLE();

		m_szName.clear();
	}
}

void* CLibrary::GetFunctionAddress( const char* const pszName ) const
{
	assert( IsLoaded() );
	assert( pszName );

	return DoGetFunctionAddress( m_hLibrary, pszName );
}

#ifdef WIN32
CLibrary::LibraryHandle_t CLibrary::DoLoad( const char* const pszFilename )
{
	return LoadLibraryA( pszFilename );
}

void CLibrary::DoFree( LibraryHandle_t hLibrary )
{
	FreeLibrary( static_cast<HMODULE>( hLibrary ) );
}

void* CLibrary::DoGetFunctionAddress( LibraryHandle_t hLibrary, const char* const pszName )
{
	return GetProcAddress( static_cast<HMODULE>( hLibrary ), pszName );
}
#else
CLibrary::LibraryHandle_t CLibrary::DoLoad( const char* const pszFilename )
{
	return dlopen( pszFilename, RTLD_NOW );
}

void CLibrary::DoFree( LibraryHandle_t hLibrary )
{
	dlclose( static_cast<void*>( hLibrary ) );
}

void* CLibrary::DoGetFunctionAddress( LibraryHandle_t hLibrary, const char* const pszName )
{
	return dlsym( static_cast<void*>( hLibrary ), pszName );
}
#endif