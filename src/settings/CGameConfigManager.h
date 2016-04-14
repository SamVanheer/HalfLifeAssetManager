#ifndef SETTINGS_CGAMECONFIGMANAGER_H
#define SETTINGS_CGAMECONFIGMANAGER_H

#include <vector>
#include <memory>

namespace settings
{
class CGameConfig;

class CGameConfigManager final : public std::enable_shared_from_this<CGameConfigManager>
{
private:
	typedef std::vector<std::shared_ptr<CGameConfig>> Configs_t;

public:
	/**
	*	Constructs an empty manager.
	*/
	CGameConfigManager();

	/**
	*	Destructor.
	*/
	~CGameConfigManager();

	/**
	*	Constructs a manager by copying from the other manager. This is a deep copy, no configs are shared.
	*	@param other Manager to copy from.
	*/
	CGameConfigManager( const CGameConfigManager& other );

	/**
	*	Sets the contents of this manager to the given one.
	*	@param other Manager to copy from.
	*	@return *this
	*/
	CGameConfigManager& operator=( const CGameConfigManager& other );

	/**
	*	Gets the list of configs. The returned list may not be modified.
	*/
	const Configs_t& GetConfigs() const { return m_Configs; }

	/**
	*	Gets the list of configs. The returned list may be modified.
	*/
	Configs_t GetConfigs() { return m_Configs; }

	/**
	*	Returns whether this manager has the given config.
	*	@param config Configuration to search for.
	*	@return true if this manager has the given config, false otherwise.
	*/
	bool HasConfig( const std::shared_ptr<const CGameConfig>& config ) const;

	/**
	*	Returns whether this manager has the given config.
	*	@param pszName Name of the configuration to search for.
	*	@return true if this manager has the given config, false otherwise.
	*/
	bool HasConfig( const char* const pszName ) const;

	/**
	*	Gets a config by name.
	*	@param pszName Name of the config to search for.
	*	@return Config, or nullptr.
	*/
	std::shared_ptr<const CGameConfig> GetConfig( const char* const pszName ) const;

	/**
	*	@copydoc GetConfig( const char* const pszName ) const
	*/
	std::shared_ptr<CGameConfig> GetConfig( const char* const pszName );

	/**
	*	Adds a config to the manager.
	*	@param config Config to add. Must be non-null, and have a unique name.
	*	@return true if the config was added, false otherwise.
	*	@see HasConfig( const char* const pszName ) const
	*/
	bool AddConfig( const std::shared_ptr<CGameConfig>& config );

	/**
	*	Removes a config from the manager. If the given config is the active config, the active config is set to nullptr.
	*	@param config Config to remove.
	*	@return true if the config was removed, false otherwise.
	*/
	bool RemoveConfig( const std::shared_ptr<CGameConfig>& config );

	/**
	*	Removes all configs. The active config is set to nullptr.
	*/
	void RemoveAllConfigs();

	/**
	*	Renames a given config to the given name. The config must be managed by this manager, and the given name must be unique.
	*	@param config Config to rename. Must be non-null.
	*	@param pszNewName New name to assign to the config.
	*	@param true if the config was renamed, false otherwise.
	*/
	bool RenameConfig( const std::shared_ptr<CGameConfig>& config, const char* const pszNewName );

	/**
	*	@see RenameConfig( std::shared_ptr<CGameConfig> config, const char* const pszNewName )
	*/
	bool RenameConfig( const char* const pszCurrentName, const char* const pszNewName );

	/**
	*	Gets the active config.
	*/
	const std::shared_ptr<const CGameConfig>& GetActiveConfig() const { return m_ActiveConfig; }

	/**
	*	@copydoc GetActiveConfig() const
	*/
	std::shared_ptr<CGameConfig> GetActiveConfig() { return m_ActiveConfig; }

	/**
	*	Sets the active config to the given config. The given config must be managed by this manager.
	*	@param config Config to set as the active config.
	*	@return true if the config was set as the active config, false otherwise.
	*/
	bool SetActiveConfig( const std::shared_ptr<CGameConfig>& config );

	/**
	*	@see SetActiveConfig( const std::shared_ptr<CGameConfig>& config )
	*/
	bool SetActiveConfig( const char* const pszName );

private:
	void Copy( const CGameConfigManager& other );

private:
	Configs_t m_Configs;

	std::shared_ptr<CGameConfig> m_ActiveConfig;
};
}

#endif //SETTINGS_CGAMECONFIGMANAGER_H