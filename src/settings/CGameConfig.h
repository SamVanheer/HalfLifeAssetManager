#ifndef SETTINGS_CGAMECONFIG_H
#define SETTINGS_CGAMECONFIG_H

#include "common/Platform.h"

namespace settings
{
class CGameConfigManager;

/**
*	Defines a single game configuration.
*	Each configuration has a unique name. This name can only be changed through the configuration manager.
*	Unmanaged configurations cannot have their name changed.
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

	const char* GetName() const { return m_szName; }

	const char* GetBasePath() const { return m_szBasePath; }

	void SetBasePath( const char* pszPath );

	const char* GetGameDir() const { return m_szGameDir; }

	void SetGameDir( const char* pszDirectory );

	const char* GetModDir() const { return m_szModDir; }

	void SetModDir( const char* pszDirectory );

private:
	friend class CGameConfigManager;

	/**
	*	Sets the name of this config. The name must not be null or empty.
	*	@param pszName New name of this config.
	*	@return true if the name has changed, false otherwise.
	*/
	bool SetName( const char* const pszName );

	void Copy( const CGameConfig& other );

private:
	/**
	*	Name of this config. Must be unique.
	*/
	char m_szName[ MAX_BUFFER_LENGTH ];

	/**
	*	Path to the base game directory, e.g. "common/Half-life"
	*/
	char m_szBasePath[ MAX_PATH_LENGTH ];

	/**
	*	Name of the game directory, e.g. "valve"
	*/
	char m_szGameDir[ MAX_PATH_LENGTH ];

	/**
	*	Name of the mod directory, e.g. "gearbox"
	*/
	char m_szModDir[ MAX_PATH_LENGTH ];
};
}

#endif //SETTINGS_CGAMECONFIG_H