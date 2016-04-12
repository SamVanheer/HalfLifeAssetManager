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
	CGameConfigManager();
	~CGameConfigManager();

	const Configs_t& GetConfigs() const { return m_Configs; }
	Configs_t GetConfigs() { return m_Configs; }

	bool HasConfig( const std::shared_ptr<const CGameConfig>& config ) const;

	bool HasConfig( const char* const pszName ) const;

	std::shared_ptr<const CGameConfig> GetConfig( const char* const pszName ) const;

	std::shared_ptr<CGameConfig> GetConfig( const char* const pszName );

	bool AddConfig( std::shared_ptr<CGameConfig> config );

	bool RemoveConfig( std::shared_ptr<CGameConfig> config );

	void RemoveAllConfigs();

	bool RenameConfig( std::shared_ptr<CGameConfig> config, const char* const pszNewName );
	bool RenameConfig( const char* const pszCurrentName, const char* const pszNewName );

	const std::shared_ptr<const CGameConfig>& GetActiveConfig() const { return m_ActiveConfig; }
	std::shared_ptr<CGameConfig> GetActiveConfig() { return m_ActiveConfig; }

	bool SetActiveConfig( const std::shared_ptr<CGameConfig>& config );
	bool SetActiveConfig( const char* const pszName );

private:
	Configs_t m_Configs;

	std::shared_ptr<CGameConfig> m_ActiveConfig;

private:
	CGameConfigManager( const CGameConfigManager& ) = delete;
	CGameConfigManager& operator=( const CGameConfigManager& ) = delete;
};
}

#endif //SETTINGS_CGAMECONFIGMANAGER_H