#include <cassert>
#include <cstring>
#include <utility>

#include "shared/Platform.h"

#ifdef WIN32
//Nothing so far
#else
#include <link.h>
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

const char* CLibrary::GetLoadErrorDescription()
{
	static char szBuffer[ 512 ];

#ifdef WIN32
	if( FormatMessageA( 
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr,
		GetLastError(),
		MAKELANGID( LANG_NEUTRAL, SUBLANG_NEUTRAL ),
		szBuffer,
		sizeof( szBuffer ),
		nullptr ) == 0 )
	{
		strcpy( szBuffer, "Unknown error" );
	}
#else
	strncpy( szBuffer, dlerror(), sizeof( szBuffer ) );
	szBuffer[ sizeof( szBuffer ) - 1 ] = '\0';
#endif

	size_t uiLength = strlen( szBuffer );

	//Strip any newlines after the message.
	if( uiLength > 0 && 
		szBuffer[ uiLength - 1 ] == '\n' )
	{
		szBuffer[ --uiLength ] = '\0';
	}

	//carriage returns too (Windows).
	if( uiLength > 0 &&
		szBuffer[ uiLength - 1 ] == '\r' )
	{
		szBuffer[ --uiLength ] = '\0';
	}

	return szBuffer;
}

const char* CLibrary::GetAbsoluteFilename() const
{
	static char szBuffer[ MAX_PATH ];

	return DoGetAbsoluteFilename( m_hLibrary, szBuffer, sizeof( szBuffer ) );
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

const char* CLibrary::DoGetAbsoluteFilename( LibraryHandle_t hLibrary, char* pszBuffer, const size_t uiBufferSize )
{
	if( GetModuleFileNameA( static_cast<HMODULE>( hLibrary ), pszBuffer, uiBufferSize ) != uiBufferSize )
		return pszBuffer;

	return "";
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

const char* CLibrary::DoGetAbsoluteFilename( LibraryHandle_t hLibrary, char* pszBuffer, const size_t uiBufferSize )
{
	struct link_map map;
	struct link_map* pMap = &map;

	if( dlinfo( static_cast<void*>( hLibrary ), RTLD_DI_LINKMAP, &pMap ) == 0 )
	{
		strncpy( pszBuffer, map.l_name, uiBufferSize );
		pszBuffer[ uiBufferSize - 1 ] = '\0';

		return pszBuffer;
	}

	return "";
}

void* CLibrary::DoGetFunctionAddress( LibraryHandle_t hLibrary, const char* const pszName )
{
	return dlsym( static_cast<void*>( hLibrary ), pszName );
}
#endif