#include <cassert>

#include "shared/Logging.h"

#include "ConfigIO.h"
#include "GameConfigIO.h"

#include "keyvalues/Keyvalues.h"

//TODO: fix this later
#include "tools/hlmv/filesystem/IFileSystem.h"

#include "utility/IOUtils.h"

#include "CBaseSettings.h"

namespace settings
{
const double CBaseSettings::DEFAULT_FPS = 30.0;

const double CBaseSettings::MIN_FPS = 15.0;

const double CBaseSettings::MAX_FPS = 500.0;

CBaseSettings::CBaseSettings( filesystem::IFileSystem* const pFileSystem )
	: m_pFileSystem( pFileSystem )
	, m_ConfigManager( std::make_shared<CGameConfigManager>() )
{
	assert( pFileSystem );
}

CBaseSettings::~CBaseSettings()
{
}

CBaseSettings::CBaseSettings( const CBaseSettings& other )
	: CBaseSettings( other.m_pFileSystem )
{
	Copy( other );
}

CBaseSettings& CBaseSettings::operator=( const CBaseSettings& other )
{
	if( this != &other )
	{
		Copy( other );
	}

	return *this;
}

void CBaseSettings::Copy( const CBaseSettings& other )
{
	//Don't copy the filesystem.

	*m_ConfigManager = *other.m_ConfigManager;
}

bool CBaseSettings::Initialize( const char* const pszFilename )
{
	assert( pszFilename && *pszFilename );

	if( !pszFilename || !( *pszFilename ) )
		return false;

	if( m_bInitialized )
		return false;

	m_bInitialized = true;

	//Load the settings, or if the file didn't exist, save settings.
	bool bResult = LoadFromFile( pszFilename );

	if( !bResult )
	{
		bResult = SaveToFile( pszFilename );

		if( !bResult )
		{
			Error( "Failed to save settings\n" );
		}
	}

	//Initialize file system based on settings.
	if( bResult )
	{
		bResult = InitializeFileSystem();

		if( !bResult )
		{
			Error( "Failed to initialize file system\n" );
		}
	}

	if( bResult )
	{
		bResult = PostInitialize( pszFilename );

		if( !bResult )
		{
			Error( "Failed to post initialize settings\n" );
		}
	}

	return bResult;
}

void CBaseSettings::Shutdown( const char* const pszFilename )
{
	assert( pszFilename && *pszFilename );

	if( !pszFilename || !( *pszFilename ) )
		return;

	if( !m_bInitialized )
		return;

	m_bInitialized = false;

	PreShutdown( pszFilename );

	if( !SaveToFile( pszFilename ) )
		Error( "Failed to save settings!\n" );
}

bool CBaseSettings::InitializeFileSystem()
{
	m_pFileSystem->RemoveAllSearchPaths();

	bool bResult = true;

	if( auto activeConfig = m_ConfigManager->GetActiveConfig() )
	{
		bResult = InitializeFileSystem( activeConfig );
	}

	return bResult;
}

bool CBaseSettings::InitializeFileSystem( const std::shared_ptr<const CGameConfig>& config )
{
	m_pFileSystem->SetBasePath( config->GetBasePath() );

	std::string szPath;

	const char* const* ppszDirectoryExts;

	const size_t uiNumExts = m_pFileSystem->GetSteamPipeDirectoryExtensions( ppszDirectoryExts );

	//Add mod dirs first, since they override game dirs.
	if( strcmp( config->GetGameDir(), config->GetModDir() ) )
	{
		for( size_t uiIndex = 0; uiIndex < uiNumExts; ++uiIndex )
		{
			m_pFileSystem->AddSearchPath((std::string{config->GetModDir()} + ppszDirectoryExts[uiIndex]).c_str());
		}
	}

	for( size_t uiIndex = 0; uiIndex < uiNumExts; ++uiIndex )
	{
		m_pFileSystem->AddSearchPath((std::string{config->GetGameDir()} + ppszDirectoryExts[uiIndex]).c_str());
	}

	return true;
}

bool CBaseSettings::LoadFromFile( const char* const pszFilename )
{
	if( !pszFilename || !( *pszFilename ) )
		return false;

	kv::Parser parser( pszFilename );

	if( !parser.HasInputData() )
		return false;

	const kv::Parser::ParseResult result = parser.Parse();

	if( result != kv::Parser::ParseResult::SUCCESS )
	{
		Error( "Error parsing settings: The error given was:\n%s\n", kv::Parser::ParseResultToString( result ) );

		return false;
	}

	return LoadFromFile( *parser.GetKeyvalues() );
}

bool CBaseSettings::SaveToFile( const char* const pszFilename )
{
	if( !pszFilename || !( *pszFilename ) )
		return false;

	kv::Writer writer( pszFilename );

	if( !writer.IsOpen() )
	{
		return false;
	}

	return SaveToFile( writer );
}

bool CBaseSettings::LoadFromFile( const kv::Block& root )
{
	return LoadCommonSettings( root ) && LoadGameConfigs( root );
}

bool CBaseSettings::SaveToFile( kv::Writer& writer )
{
	return SaveCommonSettings( writer ) && SaveGameConfigs( writer );
}

bool CBaseSettings::LoadCommonSettings( const kv::Block& root )
{
	if( auto common = root.FindFirstChild<kv::Block>( "commonSettings" ) )
	{
		if( auto cvars = common->FindFirstChild<kv::Block>( "cvars" ) )
		{
			if( !LoadArchiveCVars( *cvars ) )
				return false;
		}
	}

	return true;
}

bool CBaseSettings::SaveCommonSettings( kv::Writer& writer )
{
	writer.BeginBlock( "commonSettings" );

	if( !SaveArchiveCVars( writer, "cvars" ) )
		return false;

	writer.EndBlock();

	return !writer.ErrorOccurred();
}

bool CBaseSettings::LoadGameConfigs( const kv::Block& root )
{
	auto configs = root.FindFirstChild<kv::Block>( "gameConfigs" );

	if( configs )
	{
		const auto result = settings::LoadGameConfigs( *configs, m_ConfigManager, LoadGameConfig );

		if( result.first < result.second )
			Warning( "%u game configurations failed to load\n", result.second - result.first );
	}

	return true;
}

bool CBaseSettings::SaveGameConfigs( kv::Writer& writer )
{
	kv::Block gameConfigs( "gameConfigs" );

	settings::SaveGameConfigs( std::static_pointer_cast<const CGameConfigManager>( m_ConfigManager ), gameConfigs, SaveGameConfig );

	return writer.WriteBlock( gameConfigs );
}
}