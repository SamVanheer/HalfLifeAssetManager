#ifndef SETTINGS_CCMDLINECONFIG_H
#define SETTINGS_CCMDLINECONFIG_H

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "keyvalues/KVForward.h"

#include "CBaseConfigManager.h"

namespace settings
{
/**
*	@brief Stores command line program configuration and output file filters.
*/
class CCmdLineConfig final
{
public:
	typedef std::vector<std::pair<std::string, std::string>> Parameters_t;
	typedef std::vector<std::string> Filters_t;

	/**
	*	Name of the keyvalue block that stores command line configurations. TODO: move
	*/
	static const char* const IO_BLOCK_NAME;

public:
	/**
	*	Constructor.
	*/
	CCmdLineConfig() = default;

	/**
	*	Copy constructor.
	*/
	CCmdLineConfig( const CCmdLineConfig& other ) = default;

	/**
	*	Assignment operator.
	*/
	CCmdLineConfig& operator=( const CCmdLineConfig& other ) = default;

	/**
	*	Constructor.
	*	@param szName Name of this setting.
	*	@param parameters Command line parameters to use.
	*	@param bCopyOutputFiles Whether to copy output files or not.
	*	@param filters Filters to use.
	*/
	CCmdLineConfig( const std::string& szName, const Parameters_t& parameters, const bool bCopyOutputFiles, const Filters_t& filters );

	/**
	*	Constructor.
	*	@param szName Name of this setting.
	*	@param parameters Command line parameters to use.
	*	@param bCopyOutputFiles Whether to copy output files or not.
	*	@param filters Filters to use.
	*/
	CCmdLineConfig( std::string&& szName, Parameters_t&& parameters, const bool bCopyOutputFiles, Filters_t&& filters );

	/**
	*	Constructor.
	*	@param szName Name of this setting.
	*/
	CCmdLineConfig( const std::string& szName );

	/**
	*	Constructor.
	*	@param szName Name of this setting.
	*/
	CCmdLineConfig( std::string&& szName );

	~CCmdLineConfig() = default;

	/**
	*	@return Name of this setting.
	*/
	const std::string& GetName() const { return m_szName; }

	/**
	*	Sets the name of this setting.
	*/
	void SetName( const std::string& szName )
	{
		m_szName = szName;
	}

	/**
	*	@return List of command line parameters
	*/
	const Parameters_t& GetParameters() const { return m_Parameters; }

	/**
	*	Sets the list of command line parameters.
	*/
	void SetParameters( const Parameters_t& parameters )
	{
		m_Parameters = parameters;
	}

	/**
	*	@copydoc SetParameters( const Parameters& parameters )
	*/
	void SetParameters( Parameters_t&& parameters )
	{
		m_Parameters = std::move( parameters );
	}

	/**
	*	@return Whether or not output files should be copied.
	*/
	bool ShouldCopyOutputFiles() const { return m_bCopyOutputFiles; }

	/**
	*	Sets whether or not output files should be copied.
	*/
	void SetCopyOutputFiles( const bool bCopyOutputFiles )
	{
		m_bCopyOutputFiles = bCopyOutputFiles;
	}

	/**
	*	@return List of filters to use for output file copying.
	*/
	const Filters_t& GetFilters() const { return m_Filters; }

	/**
	*	Sets the list of filters to use for output file copying.
	*/
	void SetFilters( const Filters_t& filters )
	{
		m_Filters = filters;
	}

	/**
	*	@copydoc SetFilters( const Filters_t& filters )
	*/
	void SetFilters( Filters_t&& filters )
	{
		m_Filters = std::move( filters );
	}

private:
	std::string m_szName;

	Parameters_t m_Parameters;

	bool m_bCopyOutputFiles = false;

	Filters_t m_Filters;
};

template<>
class CConfigTraits<CCmdLineConfig> final : public CBaseConfigTraits<CCmdLineConfig, CConfigTraits<CCmdLineConfig>>
{
public:
	static const char* GetName( const CCmdLineConfig& config )
	{
		return config.GetName().c_str();
	}

	static bool SetName( CCmdLineConfig& config, const char* const pszName )
	{
		config.SetName( pszName );

		return true;
	}
};

typedef CBaseConfigManager<CCmdLineConfig> CCmdLineConfigManager;

std::shared_ptr<CCmdLineConfig> LoadCmdLineConfig( const kv::Block& kvSettings );
bool SaveCmdLineConfig( const CCmdLineConfig& settings, kv::Writer& writer );
}

#endif //SETTINGS_CCMDLINECONFIG_H