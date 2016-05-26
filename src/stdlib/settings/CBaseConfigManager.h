#ifndef SETTINGS_CBASECONFIGMANAGER_H
#define SETTINGS_CBASECONFIGMANAGER_H

#include <vector>
#include <memory>

/**
*	@defgroup Settings Settings
*
*	@{
*/

namespace settings
{
/**
*	@brief Listener for catching config events.
*/
template<typename CONFIG>
class IConfigListener
{
public:
	typedef CONFIG Config_t;

public:
	virtual ~IConfigListener() = 0;

	/**
	*	Called when the active config changes.
	*	@param oldConfig The old configuration. May be null.
	*	@param newConfig The new configuration. May be null.
	*/
	virtual void ActiveConfigChanged( const std::shared_ptr<CONFIG>& oldConfig, const std::shared_ptr<CONFIG>& newConfig ) = 0;
};

template<typename CONFIG>
inline IConfigListener<CONFIG>::~IConfigListener()
{
}

/**
*	Base class for config traits.
*/
template<typename CONFIG, typename CONFIG_TRAITS>
class CBaseConfigTraits
{
public:
	typedef CONFIG Config_t;
	typedef CONFIG_TRAITS ConfigTraits_t;

private:
	CBaseConfigTraits() = delete;
	CBaseConfigTraits( const CBaseConfigTraits& ) = delete;
	CBaseConfigTraits& operator=( const CBaseConfigTraits& ) = delete;
};

/**
*	@brief Traits class for configs, used by the config manager. Specialize this to automatically provide traits for configs classes.
*/
template<typename CONFIG>
class CConfigTraits final : public CBaseConfigTraits<CONFIG, CConfigTraits<CONFIG>>
{
public:
	static const char* GetName( const CONFIG& config )
	{
		static_assert( false, "CConfigTraits: Implement GetName" );
	}

	static bool SetName( CONFIG& config, const char* const pszName )
	{
		static_assert( false, "CConfigTraits: Implement SetName" );

		return false;
	}
};

/**
*	@brief Manages a list of configurations, as well as an active configuration.
*	@tparam CONFIG Configuration class.
*	@tparam CONFIG_TRAITS Config traits class type.
*/
template<typename CONFIG, typename CONFIG_TRAITS = CConfigTraits<CONFIG>>
class CBaseConfigManager final
{
public:
	typedef CONFIG Config_t;
	typedef CONFIG_TRAITS ConfigTraits_t;

	typedef IConfigListener<CONFIG> ConfigListener_t;

	typedef std::vector<std::shared_ptr<CONFIG>> Configs_t;

	/**
	*	Invalid config index.
	*/
	static const size_t INVALID_INDEX = static_cast<size_t>( -1 );

public:
	/**
	*	Constructs an empty manager.
	*/
	CBaseConfigManager() = default;

	/**
	*	Destructor.
	*/
	~CBaseConfigManager();

	/**
	*	Constructs a manager by copying from the other manager. This is a deep copy, no configs are shared.
	*	@param other Manager to copy from.
	*/
	CBaseConfigManager( const CBaseConfigManager& other );

	/**
	*	Sets the contents of this manager to the given one.
	*	@param other Manager to copy from.
	*	@return *this
	*/
	CBaseConfigManager& operator=( const CBaseConfigManager& other );

	/**
	*	Gets the list of configs. The returned list may not be modified.
	*/
	const Configs_t& GetConfigs() const { return m_Configs; }

	/**
	*	Returns whether this manager has the given config.
	*	@param config Configuration to search for.
	*	@return true if this manager has the given config, false otherwise.
	*/
	bool HasConfig( const std::shared_ptr<const CONFIG>& config ) const;

	/**
	*	Returns whether this manager has the given config.
	*	@param pszName Name of the configuration to search for.
	*	@return true if this manager has the given config, false otherwise.
	*/
	bool HasConfig( const char* const pszName ) const;

	/**
	*	Gets the index of the given config, or INVALID_INDEX if the config is not in the list.
	*	@param config Configuration to search for.
	*	@return Index of the config, or INVALID_INDEX if the config isn't in the list.
	*/
	size_t IndexOf( const std::shared_ptr<const CONFIG>& config ) const;

	/**
	*	Gets a config by name.
	*	@param pszName Name of the config to search for.
	*	@return Config, or nullptr.
	*/
	std::shared_ptr<const CONFIG> GetConfig( const char* const pszName ) const;

	/**
	*	@copydoc GetConfig( const char* const pszName ) const
	*/
	std::shared_ptr<CONFIG> GetConfig( const char* const pszName );

	/**
	*	Adds a config to the manager.
	*	@param config Config to add. Must be non-null, and have a unique name.
	*	@return true if the config was added, false otherwise.
	*	@see HasConfig( const char* const pszName ) const
	*/
	bool AddConfig( const std::shared_ptr<CONFIG>& config );

	/**
	*	Removes a config from the manager. If the given config is the active config, the active config is set to nullptr.
	*	@param config Config to remove.
	*	@return true if the config was removed, false otherwise.
	*/
	bool RemoveConfig( const std::shared_ptr<CONFIG>& config );

	/**
	*	Removes a config from the manager. If the given config is the active config, the active config is set to nullptr.
	*	@param pszName Name of the config to remove.
	*	@return true if the config was removed, false otherwise.
	*/
	bool RemoveConfig( const char* const pszName );

	/**
	*	Removes all configs. The active config is set to nullptr.
	*/
	void RemoveAllConfigs();

	/**
	*	Returns whether or not the given config can be renamed to the given name.
	*	@param config Config to rename.
	*	@param pszNewName New name.
	*	@return Whether or not the given config can be renamed to the given name.
	*/
	bool CanRenameConfig( const std::shared_ptr<CONFIG>& config, const char* const pszNewName ) const;

	/**
	*	@see CanRenameConfig( const std::shared_ptr<CONFIG>& config, const char* const pszNewName )
	*/
	bool CanRenameConfig( const char* const pszCurrentName, const char* const pszNewName ) const;

	/**
	*	Renames a given config to the given name. The config must be managed by this manager, and the given name must be unique.
	*	@param config Config to rename. Must be non-null.
	*	@param pszNewName New name to assign to the config.
	*	@return true if the config was renamed, false otherwise.
	*/
	bool RenameConfig( const std::shared_ptr<CONFIG>& config, const char* const pszNewName );

	/**
	*	@see RenameConfig( const std::shared_ptr<CONFIG>& config, const char* const pszNewName )
	*/
	bool RenameConfig( const char* const pszCurrentName, const char* const pszNewName );

	/**
	*	Gets the active config.
	*	@return The active config.
	*/
	const std::shared_ptr<const CONFIG>& GetActiveConfig() const { return m_ActiveConfig; }

	/**
	*	@copydoc GetActiveConfig() const
	*/
	std::shared_ptr<CONFIG> GetActiveConfig() { return m_ActiveConfig; }

	/**
	*	Sets the active config to the given config. The given config must be managed by this manager.
	*	@param config Config to set as the active config.
	*	@return true if the config was set as the active config, false otherwise.
	*/
	bool SetActiveConfig( const std::shared_ptr<CONFIG>& config );

	/**
	*	Sets the active config to the given config.
	*	@param pszName Name of the config to set.
	*	@return true if the config was set as the active config, false otherwise.
	*	@see SetActiveConfig( const std::shared_ptr<CONFIG>& config )
	*/
	bool SetActiveConfig( const char* const pszName );

	/**
	*	Clears the active config.
	*/
	void ClearActiveConfig();

	/**
	*	Gets the config listener.
	*	@return Config listener.
	*/
	ConfigListener_t* GetListener() { return m_pListener; }

	/**
	*	Sets the config listener.
	*	@param pListener Listener to set. May be null.
	*/
	void SetListener( ConfigListener_t* pListener )
	{
		m_pListener = pListener;
	}

private:
	void Copy( const CBaseConfigManager& other );

	bool DoSetActiveConfig( const std::shared_ptr<CONFIG>& config );

private:
	Configs_t m_Configs;

	std::shared_ptr<CONFIG> m_ActiveConfig;

	ConfigListener_t* m_pListener = nullptr;
};

template<typename CONFIG, typename CONFIG_TRAITS>
CBaseConfigManager<CONFIG, CONFIG_TRAITS>::~CBaseConfigManager()
{
	m_pListener = nullptr;

	RemoveAllConfigs();
}

template<typename CONFIG, typename CONFIG_TRAITS>
CBaseConfigManager<CONFIG, CONFIG_TRAITS>::CBaseConfigManager( const CBaseConfigManager<CONFIG, CONFIG_TRAITS>& other )
{
	Copy( other );
}

template<typename CONFIG, typename CONFIG_TRAITS>
CBaseConfigManager<CONFIG, CONFIG_TRAITS>& CBaseConfigManager<CONFIG, CONFIG_TRAITS>::operator=( const CBaseConfigManager<CONFIG, CONFIG_TRAITS>& other )
{
	if( this != &other )
	{
		Copy( other );
	}

	return *this;
}

template<typename CONFIG, typename CONFIG_TRAITS>
void CBaseConfigManager<CONFIG, CONFIG_TRAITS>::Copy( const CBaseConfigManager<CONFIG, CONFIG_TRAITS>& other )
{
	RemoveAllConfigs();

	for( const auto& config : other.m_Configs )
	{
		m_Configs.push_back( std::make_shared<CONFIG>( *config ) );
	}

	//Don't pass the actual config object, since it differs from our copy.
	if( other.m_ActiveConfig )
		SetActiveConfig( ConfigTraits_t::GetName( *other.m_ActiveConfig ) );
	else
		ClearActiveConfig();

	//Don't copy the listener.
}

template<typename CONFIG, typename CONFIG_TRAITS>
bool CBaseConfigManager<CONFIG, CONFIG_TRAITS>::HasConfig( const std::shared_ptr<const CONFIG>& config ) const
{
	if( !config )
		return false;

	auto it = std::find( m_Configs.begin(), m_Configs.end(), config );

	return it != m_Configs.end();
}

template<typename CONFIG, typename CONFIG_TRAITS>
bool CBaseConfigManager<CONFIG, CONFIG_TRAITS>::HasConfig( const char* const pszName ) const
{
	return GetConfig( pszName ) != nullptr;
}

template<typename CONFIG, typename CONFIG_TRAITS>
size_t CBaseConfigManager<CONFIG, CONFIG_TRAITS>::IndexOf( const std::shared_ptr<const CONFIG>& config ) const
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

template<typename CONFIG, typename CONFIG_TRAITS>
std::shared_ptr<const CONFIG> CBaseConfigManager<CONFIG, CONFIG_TRAITS>::GetConfig( const char* const pszName ) const
{
	return const_cast<CBaseConfigManager*>( this )->GetConfig( pszName );
}

template<typename CONFIG, typename CONFIG_TRAITS>
std::shared_ptr<CONFIG> CBaseConfigManager<CONFIG, CONFIG_TRAITS>::GetConfig( const char* const pszName )
{
	if( !pszName || !( *pszName ) )
		return nullptr;

	auto it = std::find_if( m_Configs.begin(), m_Configs.end(),
							[ & ]( const std::shared_ptr<const CONFIG>& config )
	{
		return strcmp( pszName, ConfigTraits_t::GetName( *config ) ) == 0;
	}
	);

	return it != m_Configs.end() ? *it : nullptr;
}

template<typename CONFIG, typename CONFIG_TRAITS>
bool CBaseConfigManager<CONFIG, CONFIG_TRAITS>::AddConfig( const std::shared_ptr<CONFIG>& config )
{
	if( !config )
		return false;

	if( !( *ConfigTraits_t::GetName( *config ) ) )
		return false;

	if( HasConfig( config ) )
		return false;

	if( HasConfig( ConfigTraits_t::GetName( *config ) ) )
		return false;

	m_Configs.push_back( config );

	return true;
}

template<typename CONFIG, typename CONFIG_TRAITS>
bool CBaseConfigManager<CONFIG, CONFIG_TRAITS>::RemoveConfig( const std::shared_ptr<CONFIG>& config )
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

template<typename CONFIG, typename CONFIG_TRAITS>
bool CBaseConfigManager<CONFIG, CONFIG_TRAITS>::RemoveConfig( const char* const pszName )
{
	return RemoveConfig( GetConfig( pszName ) );
}

template<typename CONFIG, typename CONFIG_TRAITS>
void CBaseConfigManager<CONFIG, CONFIG_TRAITS>::RemoveAllConfigs()
{
	m_Configs.clear();

	if( m_ActiveConfig )
		m_ActiveConfig.reset();
}

template<typename CONFIG, typename CONFIG_TRAITS>
bool CBaseConfigManager<CONFIG, CONFIG_TRAITS>::CanRenameConfig( const std::shared_ptr<CONFIG>& config, const char* const pszNewName ) const
{
	if( !config )
		return false;

	if( !pszNewName || !( *pszNewName ) )
		return false;

	if( !HasConfig( config ) )
		return false;

	auto other = GetConfig( pszNewName );

	//Rename to same name.
	if( other && other != config )
		return false;

	return true;
}

template<typename CONFIG, typename CONFIG_TRAITS>
bool CBaseConfigManager<CONFIG, CONFIG_TRAITS>::CanRenameConfig( const char* const pszCurrentName, const char* const pszNewName ) const
{
	if( !pszCurrentName || !( *pszCurrentName ) )
		return false;

	if( !pszNewName || !( *pszNewName ) )
		return false;

	auto config = GetConfig( pszCurrentName );

	if( !config )
		return false;

	auto other = GetConfig( pszNewName );

	//Rename to same name.
	if( other && other != config )
		return false;

	return true;
}

template<typename CONFIG, typename CONFIG_TRAITS>
bool CBaseConfigManager<CONFIG, CONFIG_TRAITS>::RenameConfig( const std::shared_ptr<CONFIG>& config, const char* const pszNewName )
{
	if( !CanRenameConfig( config, pszNewName ) )
		return false;

	return ConfigTraits_t::SetName( *config, pszNewName );
}

template<typename CONFIG, typename CONFIG_TRAITS>
bool CBaseConfigManager<CONFIG, CONFIG_TRAITS>::RenameConfig( const char* const pszCurrentName, const char* const pszNewName )
{
	if( !CanRenameConfig( pszCurrentName, pszNewName ) )
		return false;

	return ConfigTraits_t::SetName( *GetConfig( pszCurrentName ), pszNewName );
}

template<typename CONFIG, typename CONFIG_TRAITS>
bool CBaseConfigManager<CONFIG, CONFIG_TRAITS>::SetActiveConfig( const std::shared_ptr<CONFIG>& config )
{
	if( !config )
		return false;

	if( !HasConfig( config ) )
		return false;

	return DoSetActiveConfig( config );
}

template<typename CONFIG, typename CONFIG_TRAITS>
bool CBaseConfigManager<CONFIG, CONFIG_TRAITS>::SetActiveConfig( const char* const pszName )
{
	auto config = GetConfig( pszName );

	if( !config )
		return false;

	return DoSetActiveConfig( config );
}

template<typename CONFIG, typename CONFIG_TRAITS>
void CBaseConfigManager<CONFIG, CONFIG_TRAITS>::ClearActiveConfig()
{
	if( !m_ActiveConfig )
		return;

	DoSetActiveConfig( nullptr );
}

template<typename CONFIG, typename CONFIG_TRAITS>
bool CBaseConfigManager<CONFIG, CONFIG_TRAITS>::DoSetActiveConfig( const std::shared_ptr<CONFIG>& config )
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

/** @} */

#endif //SETTINGS_CBASECONFIGMANAGER_H