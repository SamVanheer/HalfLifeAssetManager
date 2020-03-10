#include <memory>
#include <cstring>
#include <stdexcept>

#include "CGameConfig.h"

namespace settings
{
CGameConfig::CGameConfig( const char* const pszName )
{
	if( !SetName( pszName ) )
	{
		throw std::invalid_argument( "Name is invalid!" );
	}
}

CGameConfig::CGameConfig( const char* const pszName, const CGameConfig& other )
	: CGameConfig( pszName )
{
	m_szBasePath = other.m_szBasePath;
	m_szGameDir = other.m_szGameDir;
	m_szModDir = other.m_szModDir;
}

CGameConfig::CGameConfig( const CGameConfig& other )
{
	Copy( other );
}

CGameConfig& CGameConfig::operator=( const CGameConfig& other )
{
	if( this != &other )
	{
		Copy( other );
	}

	return *this;
}

void CGameConfig::Copy( const CGameConfig& other )
{
	//The name is already valid since the other instance validated it.
	m_szName = other.m_szName;
	m_szBasePath = other.m_szBasePath;
	m_szGameDir = other.m_szGameDir;
	m_szModDir = other.m_szModDir;
}

bool CGameConfig::SetName( const char* const pszName )
{
	if( !pszName || !( *pszName ) )
		return false;

	m_szName = pszName;

	return true;
}

void CGameConfig::SetBasePath( const char* pszPath )
{
	if( !pszPath )
		pszPath = "";

	m_szBasePath = pszPath;
}

void CGameConfig::SetGameDir( const char* pszDirectory )
{
	if( !pszDirectory )
		pszDirectory = "";

	m_szGameDir = pszDirectory;
}

void CGameConfig::SetModDir( const char* pszDirectory )
{
	if( !pszDirectory )
		pszDirectory = "";

	m_szModDir = pszDirectory;
}
}