#include <memory>
#include <cstring>
#include <stdexcept>

#include "CGameConfig.h"

namespace settings
{
CGameConfig::CGameConfig( const char* const pszName )
{
	memset( m_szName, 0, sizeof( m_szName ) );
	memset( m_szBasePath, 0, sizeof( m_szBasePath ) );
	memset( m_szGameDir, 0, sizeof( m_szGameDir ) );
	memset( m_szModDir, 0, sizeof( m_szModDir ) );

	if( !SetName( pszName ) )
	{
		throw std::invalid_argument( "Name is invalid!" );
	}
}

CGameConfig::CGameConfig( const char* const pszName, const CGameConfig& other )
	: CGameConfig( pszName )
{
	strcpy( m_szBasePath, other.m_szBasePath );
	strcpy( m_szGameDir, other.m_szGameDir );
	strcpy( m_szModDir, other.m_szModDir );
}

bool CGameConfig::SetName( const char* const pszName )
{
	if( !pszName || !( *pszName ) )
		return false;

	strncpy( m_szName, pszName, sizeof( m_szName ) );
	m_szName[ sizeof( m_szName ) - 1 ] = '\0';

	return true;
}

void CGameConfig::SetBasePath( const char* pszPath )
{
	if( !pszPath )
		pszPath = "";

	strncpy( m_szBasePath, pszPath, sizeof( m_szBasePath ) );
	m_szBasePath[ sizeof( m_szBasePath ) - 1 ] = '\0';
}

void CGameConfig::SetGameDir( const char* pszDirectory )
{
	if( !pszDirectory )
		pszDirectory = "";

	strncpy( m_szGameDir, pszDirectory, sizeof( m_szGameDir ) );
	m_szGameDir[ sizeof( m_szGameDir ) - 1 ] = '\0';
}

void CGameConfig::SetModDir( const char* pszDirectory )
{
	if( !pszDirectory )
		pszDirectory = "";

	strncpy( m_szModDir, pszDirectory, sizeof( m_szModDir ) );
	m_szModDir[ sizeof( m_szModDir ) - 1 ] = '\0';
}
}