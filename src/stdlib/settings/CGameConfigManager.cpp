#include <algorithm>

#include "CGameConfig.h"

#include "CGameConfigManager.h"

namespace settings
{
CGameConfigManager::CGameConfigManager()
{
}

CGameConfigManager::~CGameConfigManager()
{
	m_pListener = nullptr;

	RemoveAllConfigs();
}

CGameConfigManager::CGameConfigManager( const CGameConfigManager& other )
{
	Copy( other );
}

CGameConfigManager& CGameConfigManager::operator=( const CGameConfigManager& other )
{
	if( this != &other )
	{
		Copy( other );
	}

	return *this;
}

void CGameConfigManager::Copy( const CGameConfigManager& other )
{
	RemoveAllConfigs();

	for( const auto& config : other.m_Configs )
	{
		m_Configs.push_back( std::make_shared<CGameConfig>( *config ) );
	}

	//Don't pass the actual config object, since it differs from our copy.
	if( other.m_ActiveConfig )
		SetActiveConfig( other.m_ActiveConfig->GetName() );
	else
		ClearActiveConfig();

	//Don't copy the listener.
}

bool CGameConfigManager::HasConfig( const std::shared_ptr<const CGameConfig>& config ) const
{
	if( !config )
		return false;

	auto it = std::find( m_Configs.begin(), m_Configs.end(), config );

	return it != m_Configs.end();
}

bool CGameConfigManager::HasConfig( const char* const pszName ) const
{
	return GetConfig( pszName ) != nullptr;
}

size_t CGameConfigManager::IndexOf( const std::shared_ptr<const CGameConfig>& config ) const
{
	if( !config )
		return INVALID_INDEX;

	auto it = std::find( m_Configs.begin(), m_Configs.end(), config );

	if( it != m_Configs.end() )
	{
		return it - m_Configs.begin();
	}

	return INVALID_INDEX;
}

std::shared_ptr<const CGameConfig> CGameConfigManager::GetConfig( const char* const pszName ) const
{
	return const_cast<CGameConfigManager*>( this )->GetConfig( pszName );
}

std::shared_ptr<CGameConfig> CGameConfigManager::GetConfig( const char* const pszName )
{
	if( !pszName || !( *pszName ) )
		return nullptr;

	auto it = std::find_if( m_Configs.begin(), m_Configs.end(), 
		[ & ]( const std::shared_ptr<const CGameConfig>& config )
		{
			return strcmp( pszName, config->GetName() ) == 0;
		}
	);

	return it != m_Configs.end() ? *it : nullptr;
}

bool CGameConfigManager::AddConfig( const std::shared_ptr<CGameConfig>& config )
{
	if( !config )
		return false;

	if( !( *config->GetName() ) )
		return false;

	if( HasConfig( config ) )
		return false;

	if( HasConfig( config->GetName() ) )
		return false;

	m_Configs.push_back( config );

	return true;
}

bool CGameConfigManager::RemoveConfig( const std::shared_ptr<CGameConfig>& config )
{
	if( !config )
		return false;

	auto it = std::find( m_Configs.begin(), m_Configs.end(), config );

	if( it != m_Configs.end() )
	{
		m_Configs.erase( it );

		if( m_ActiveConfig == config )
			m_ActiveConfig.reset();

		return true;
	}

	return false;
}

void CGameConfigManager::RemoveAllConfigs()
{
	m_Configs.clear();

	if( m_ActiveConfig )
		m_ActiveConfig.reset();
}

bool CGameConfigManager::RenameConfig( const std::shared_ptr<CGameConfig>& config, const char* const pszNewName )
{
	if( !config )
		return false;

	if( !pszNewName || !( *pszNewName ) )
		return false;

	if( !HasConfig( config ) )
		return false;

	if( HasConfig( pszNewName ) )
		return false;

	return config->SetName( pszNewName );
}

bool CGameConfigManager::RenameConfig( const char* const pszCurrentName, const char* const pszNewName )
{
	if( !pszCurrentName || !( *pszCurrentName ) )
		return false;

	if( !pszNewName || !( *pszNewName ) )
		return false;

	auto config = GetConfig( pszCurrentName );

	if( !config )
		return false;

	if( HasConfig( pszNewName ) )
		return false;

	return config->SetName( pszNewName );
}

bool CGameConfigManager::SetActiveConfig( const std::shared_ptr<CGameConfig>& config )
{
	if( !config )
		return false;

	if( !HasConfig( config ) )
		return false;

	return DoSetActiveConfig( config );
}

bool CGameConfigManager::SetActiveConfig( const char* const pszName )
{
	auto config = GetConfig( pszName );

	if( !config )
		return false;

	return DoSetActiveConfig( config );
}

void CGameConfigManager::ClearActiveConfig()
{
	if( !m_ActiveConfig )
		return;

	DoSetActiveConfig( nullptr );
}

bool CGameConfigManager::DoSetActiveConfig( const std::shared_ptr<CGameConfig>& config )
{
	//Nothing to do.
	if( m_ActiveConfig == config )
		return true;

	auto oldConfig = m_ActiveConfig;

	m_ActiveConfig = config;

	if( m_pListener )
		m_pListener->ActiveConfigChanged( oldConfig, m_ActiveConfig );

	return true;
}
}