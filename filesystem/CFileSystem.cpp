#include <cstring>
#include <cstdio>

#include "common/Logging.h"

#include "CFileSystem.h"

namespace filesystem
{
CFileSystem* CFileSystem::m_pInstance = nullptr;

CFileSystem& CFileSystem::CreateInstance()
{
	if( m_pInstance )
	{
		Warning( "CFileSystem::CreateInstance called multiple times!\n" );
		return *m_pInstance;
	}

	m_pInstance = new CFileSystem();

	return *m_pInstance;
}

void CFileSystem::DestroyInstance()
{
	if( m_pInstance )
	{
		delete m_pInstance;
		m_pInstance = nullptr;
	}
}

bool CFileSystem::InstanceExists()
{
	return m_pInstance != nullptr;
}

CFileSystem& CFileSystem::GetInstance()
{
	return *m_pInstance;
}

CFileSystem::CFileSystem()
{
	memset( m_szBasePath, 0, sizeof( m_szBasePath ) );
}

CFileSystem::~CFileSystem()
{
}

bool CFileSystem::Initialize()
{
	SetBasePath( "." );

	return true;
}

void CFileSystem::Shutdown()
{
	RemoveAllSearchPaths();
}

const char* CFileSystem::GetBasePath() const
{
	return m_szBasePath;
}

void CFileSystem::SetBasePath( const char* const pszPath )
{
	if( !pszPath || !( *pszPath ) )
		return;

	strncpy( m_szBasePath, pszPath, sizeof( m_szBasePath ) );
	m_szBasePath[ sizeof( m_szBasePath ) - 1 ] = '\0';
}

bool CFileSystem::HasSearchPath( const char* const pszPath ) const
{
	if( !pszPath || !( *pszPath ) )
		return false;

	for( const auto& path : m_SearchPaths )
	{
		if( strcmp( path.szPath, pszPath ) == 0 )
			return true;
	}

	return false;
}

void CFileSystem::AddSearchPath( const char* const pszPath )
{
	if( !pszPath || !( *pszPath ) )
		return;

	if( HasSearchPath( pszPath ) )
		return;

	SearchPath_t path;

	strncpy( path.szPath, pszPath, sizeof( path.szPath ) );

	path.szPath[ sizeof( path.szPath ) - 1 ] = '\0';

	m_SearchPaths.push_back( path );
}

void CFileSystem::RemoveSearchPath( const char* const pszPath )
{
	if( !pszPath || !( *pszPath ) )
		return;

	for( auto it = m_SearchPaths.begin(); it != m_SearchPaths.end(); ++it )
	{
		if( strcmp( ( *it ).szPath, pszPath ) == 0 )
		{
			m_SearchPaths.erase( it );
			return;
		}
	}
}

void CFileSystem::RemoveAllSearchPaths()
{
	m_SearchPaths.clear();
}

bool CFileSystem::GetRelativePath( const char* const pszFilename, char* pszOutPath, const size_t uiBufferSize )
{
	if( !pszFilename || !( *pszFilename ) )
		return false;

	if( !pszOutPath || !uiBufferSize )
		return false;

	char szCompletePath[ MAX_PATH_LENGTH ];

	for( const auto& path : m_SearchPaths )
	{
		const int iRet = snprintf( szCompletePath, sizeof( szCompletePath ), "%s/%s/%s", m_szBasePath, path.szPath, pszFilename );

		if( iRet < 0 || static_cast<size_t>( iRet ) >= sizeof( szCompletePath ) )
			continue;

		if( FILE* pFile = fopen( szCompletePath, "r" ) )
		{
			fclose( pFile );

			//Buffer too small
			if( static_cast<size_t>( iRet ) >= uiBufferSize )
			{
				pszOutPath[ 0 ] = '\0';
				return true;
			}

			strncpy( pszOutPath, szCompletePath, uiBufferSize );
			pszOutPath[ uiBufferSize - 1 ] = '\0';

			return true;
		}
	}

	//TODO: refactor
	const int iRet = snprintf( szCompletePath, sizeof( szCompletePath ), "%s/%s", m_szBasePath, pszFilename );

	if( iRet < 0 || static_cast<size_t>( iRet ) >= sizeof( szCompletePath ) )
		return false;

	if( FILE* pFile = fopen( szCompletePath, "r" ) )
	{
		fclose( pFile );

		//Buffer too small
		if( static_cast<size_t>( iRet ) >= uiBufferSize )
		{
			pszOutPath[ 0 ] = '\0';
			return true;
		}

		strncpy( pszOutPath, szCompletePath, uiBufferSize );
		pszOutPath[ uiBufferSize - 1 ] = '\0';

		return true;
	}

	return false;
}
}