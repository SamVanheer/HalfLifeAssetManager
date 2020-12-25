#include <cstring>
#include <cstdio>

#include "filesystem/FileSystem.hpp"

#include "shared/Logging.hpp"
#include "shared/Utility.hpp"
#include "utility/IOUtils.hpp"
#include "utility/StringUtils.hpp"

namespace filesystem
{
FileSystem::FileSystem()
{
	memset( m_szBasePath, 0, sizeof( m_szBasePath ) );
}

FileSystem::~FileSystem()
{
}

bool FileSystem::Initialize()
{
	SetBasePath( "." );

	return true;
}

void FileSystem::Shutdown()
{
	RemoveAllSearchPaths();
}

const char* FileSystem::GetBasePath() const
{
	return m_szBasePath;
}

void FileSystem::SetBasePath( const char* const pszPath )
{
	if( !pszPath || !( *pszPath ) )
		return;

	strncpy( m_szBasePath, pszPath, sizeof( m_szBasePath ) );
	m_szBasePath[ sizeof( m_szBasePath ) - 1 ] = '\0';
}

bool FileSystem::HasSearchPath( const char* const pszPath ) const
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

void FileSystem::AddSearchPath( const char* const pszPath )
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

void FileSystem::RemoveSearchPath( const char* const pszPath )
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

void FileSystem::RemoveAllSearchPaths()
{
	m_SearchPaths.clear();
}

bool FileSystem::CheckFileExists( const char* const pszCompletePath, const size_t uiLength, char* pszOutPath, size_t uiBufferSize ) const
{
	if( FileExists( pszCompletePath ) )
	{
		//Buffer too small
		if( uiLength >= uiBufferSize )
		{
			pszOutPath[ 0 ] = '\0';
			return true;
		}

		strncpy( pszOutPath, pszCompletePath, uiBufferSize );
		pszOutPath[ uiBufferSize - 1 ] = '\0';

		return true;
	}

	return false;
}

bool FileSystem::GetRelativePath( const char* const pszFilename, char* pszOutPath, const size_t uiBufferSize )
{
	if( !pszFilename || !( *pszFilename ) )
		return false;

	if( !pszOutPath || !uiBufferSize )
		return false;

	char szCompletePath[ MAX_PATH_LENGTH ];

	for( const auto& path : m_SearchPaths )
	{
		const int iRet = snprintf( szCompletePath, sizeof( szCompletePath ), "%s/%s/%s", m_szBasePath, path.szPath, pszFilename );

		if( !PrintfSuccess( iRet, sizeof( szCompletePath ) ) )
			continue;

		if( FileExists( szCompletePath ) )
		{
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

		if( CheckFileExists( szCompletePath, static_cast<size_t>( iRet ), pszOutPath, uiBufferSize ) )
			return true;
	}

	const int iRet = snprintf( szCompletePath, sizeof( szCompletePath ), "%s/%s", m_szBasePath, pszFilename );

	if( !PrintfSuccess( iRet, sizeof( szCompletePath ) ) )
		return false;

	return CheckFileExists( szCompletePath, static_cast<size_t>( iRet ), pszOutPath, uiBufferSize );
}

bool FileSystem::FileExists( const char* const pszFilename ) const
{
	if( !pszFilename || !( *pszFilename ) )
		return false;

	if( FILE* pFile = utf8_fopen( pszFilename, "r" ) )
	{
		fclose( pFile );

		return true;
	}

	return false;
}
}