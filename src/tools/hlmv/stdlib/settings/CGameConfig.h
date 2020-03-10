#ifndef SETTINGS_CGAMECONFIG_H
#define SETTINGS_CGAMECONFIG_H

#include <string>

#include "CBaseConfigManager.h"

/**
*	@defgroup GameConfig Game configurations
*
*	Game configuration data.
*
*	@{
*/

namespace settings
{
/**
*	@brief Defines a single game configuration.
*
*	Each configuration has a unique name. This name can only be changed through the configuration manager.
*	Unmanaged configurations cannot have their name changed.
*	@see filesystem::IFileSystem
*/
class CGameConfig final
{
public:
	/**
	*	Constructs an empty config with the given name.
	*	@param pszName Name of this config.
	*	@throws std::invalid_argument If the name is null or empty.
	*/
	CGameConfig( const char* const pszName );

	/**
	*	Constructs a config from another, using the specified name.
	*	@param pszName Name of this config.
	*	@param other Config to copy from.
	*	@throws std::invalid_argument If the name is null or empty.
	*/
	CGameConfig( const char* const pszName, const CGameConfig& other );

	/**
	*	Constructs a config from another.
	*	@param other Config to copy from.
	*/
	CGameConfig( const CGameConfig& other );

	/**
	*	Sets the contents of this config to the given config.
	*	@param other Config to copy from.
	*	@return *this
	*/
	CGameConfig& operator=( const CGameConfig& other );

	/**
	*	Gets the name of this configuration.
	*	@return Name.
	*/
	const char* GetName() const { return m_szName.c_str(); }

	/**
	*	Sets the name of this config. The name must not be null or empty.
	*	@param pszName New name of this config.
	*	@return true if the name has changed, false otherwise.
	*/
	bool SetName( const char* const pszName );

	/**
	*	Gets the base path for this configuration.
	*	@return Base path.
	*	@see filesystem::IFileSystem
	*/
	const char* GetBasePath() const { return m_szBasePath.c_str(); }

	/**
	*	Sets the base path.
	*	@param pszPath Base path.
	*	@see filesystem::IFileSystem
	*/
	void SetBasePath( const char* pszPath );

	/**
	*	Gets the game directory for this configuration.
	*	@return Game directory.
	*/
	const char* GetGameDir() const { return m_szGameDir.c_str(); }

	/**
	*	Sets the game directory.
	*	@param pszDirectory Game directory.
	*/
	void SetGameDir( const char* pszDirectory );

	/**
	*	Gets the mod directory for this configuration.
	*	@return Mod directory.
	*/
	const char* GetModDir() const { return m_szModDir.c_str(); }

	/**
	*	Sets the mod directory.
	*	@param pszDirectory Mod directory.
	*/
	void SetModDir( const char* pszDirectory );

private:

	/**
	*	Copies the given configuration's settings into this one.
	*	@param other Configuration to copy from.
	*/
	void Copy( const CGameConfig& other );

private:
	/**
	*	Name of this config. Must be unique.
	*/
	std::string m_szName;

	/**
	*	Path to the base game directory, e.g. "C:\Program Files (x86)\Steam\steamapps\common\Half-life"
	*/
	std::string m_szBasePath;

	/**
	*	Name of the game directory, e.g. "valve"
	*/
	std::string m_szGameDir;

	/**
	*	Name of the mod directory, e.g. "gearbox"
	*/
	std::string m_szModDir;
};

/**
*	@brief Specialization for CGameConfig.
*	@see CConfigTraits
*	@see CGameConfig
*/
template<>
class CConfigTraits<CGameConfig> final : public CBaseConfigTraits<CGameConfig, CConfigTraits<CGameConfig>>
{
public:
	static const char* GetName( const CGameConfig& config )
	{
		return config.GetName();
	}

	static bool SetName( CGameConfig& config, const char* const pszName )
	{
		return config.SetName( pszName );
	}
};

typedef CBaseConfigManager<CGameConfig> CGameConfigManager;
}

/** @} */

#endif //SETTINGS_CGAMECONFIG_H