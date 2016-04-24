#include <cassert>

#include "common/Logging.h"

#include "CGameConfig.h"
#include "CGameConfigManager.h"

#include "GameConfigIO.h"

#include "keyvalues/Keyvalues.h"

#include "filesystem/FileSystemConstants.h"
#include "filesystem/CFileSystem.h"

#include "utility/CString.h"
#include "utility/IOUtils.h"

#include "CBaseSettings.h"

namespace settings
{
const double CBaseSettings::DEFAULT_FPS = 30.0;

const double CBaseSettings::MIN_FPS = 15.0;

//wxTimer seems to have issues going higher than ~64 FPS. Might be good to use a more game engine like main loop instead of a timer.
const double CBaseSettings::MAX_FPS = 60.0; //500.0;

CBaseSettings::CBaseSettings()
	: m_ConfigManager( std::make_shared<CGameConfigManager>() )
{
}

CBaseSettings::~CBaseSettings()
{
}

CBaseSettings::CBaseSettings( const CBaseSettings& other )
	: CBaseSettings()
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
	*m_ConfigManager = *other.m_ConfigManager;

	SetFPS( other.GetFPS() );
}

void CBaseSettings::SetFPS( const double flFPS )
{
	if( flFPS == m_flFPS )
		return;

	const double flOldFPS = m_flFPS;

	m_flFPS = clamp( flFPS, MIN_FPS, MAX_FPS );

	if( m_pListener )
		m_pListener->FPSChanged( flOldFPS, m_flFPS );
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
	fileSystem().RemoveAllSearchPaths();

	bool bResult = true;

	if( auto activeConfig = m_ConfigManager->GetActiveConfig() )
	{
		bResult = InitializeFileSystem( activeConfig );
	}

	return bResult;
}

bool CBaseSettings::InitializeFileSystem( const std::shared_ptr<const CGameConfig>& config )
{
	fileSystem().SetBasePath( config->GetBasePath() );

	CString szPath;

	//Add mod dirs first, since they override game dirs.
	if( strcmp( config->GetGameDir(), config->GetModDir() ) )
	{
		for( size_t uiIndex = 0; uiIndex < filesystem::NUM_STEAMPIPE_DIRECTORY_EXTS; ++uiIndex )
		{
			szPath.Format( "%s%s", config->GetModDir(), filesystem::STEAMPIPE_DIRECTORY_EXTS[ uiIndex ] );

			fileSystem().AddSearchPath( szPath.CStr() );
		}
	}

	for( size_t uiIndex = 0; uiIndex < filesystem::NUM_STEAMPIPE_DIRECTORY_EXTS; ++uiIndex )
	{
		szPath.Format( "%s%s", config->GetGameDir(), filesystem::STEAMPIPE_DIRECTORY_EXTS[ uiIndex ] );

		fileSystem().AddSearchPath( szPath.CStr() );
	}

	return true;
}

bool CBaseSettings::LoadFromFile( const char* const pszFilename )
{
	if( !pszFilename || !( *pszFilename ) )
		return false;

	CKeyvaluesParser parser( pszFilename );

	if( !parser.HasInputData() )
		return false;

	const CKeyvaluesParser::ParseResult result = parser.Parse();

	if( result != CKeyvaluesParser::Success )
	{
		return false;
	}

	return LoadFromFile( *parser.GetKeyvalues() );
}

bool CBaseSettings::SaveToFile( const char* const pszFilename )
{
	if( !pszFilename || !( *pszFilename ) )
		return false;

	CKeyvaluesWriter writer( pszFilename );

	if( !writer.IsOpen() )
	{
		return false;
	}

	return SaveToFile( writer );
}

bool CBaseSettings::LoadFromFile( const CKvBlockNode& root )
{
	return LoadCommonSettings( root ) && LoadGameConfigs( root );
}

bool CBaseSettings::SaveToFile( CKeyvaluesWriter& writer )
{
	return SaveCommonSettings( writer ) && SaveGameConfigs( writer );
}

bool CBaseSettings::LoadCommonSettings( const CKvBlockNode& root )
{
	if( auto common = root.FindFirstChild<CKvBlockNode>( "commonSettings" ) )
	{
		if( auto fps = common->FindFirstChild<CKeyvalue>( "fps" ) )
		{
			SetFPS( strtod( fps->GetValue().CStr(), nullptr ) );
		}

		if( auto cvars = common->FindFirstChild<CKvBlockNode>( "cvars" ) )
		{
			if( !LoadArchiveCVars( *cvars ) )
				return false;
		}
	}

	return true;
}

bool CBaseSettings::SaveCommonSettings( CKeyvaluesWriter& writer )
{
	writer.BeginBlock( "commonSettings" );

	char szBuffer[ MAX_BUFFER_LENGTH ];

	if( !PrintfSuccess( snprintf( szBuffer, sizeof( szBuffer ), "%f", GetFPS() ), sizeof( szBuffer ) ) )
		return false;

	writer.WriteKeyvalue( "fps", szBuffer );

	if( !SaveArchiveCVars( writer, "cvars" ) )
		return false;

	writer.EndBlock();

	return !writer.ErrorOccurred();
}

bool CBaseSettings::LoadGameConfigs( const CKvBlockNode& root )
{
	auto configs = root.FindFirstChild<CKvBlockNode>( "gameConfigs" );

	if( configs )
	{
		const auto result = settings::LoadGameConfigs( *configs, m_ConfigManager );

		if( result.first < result.second )
			Warning( "%u game configurations failed to load\n", result.second - result.first );
	}

	return true;
}

bool CBaseSettings::SaveGameConfigs( CKeyvaluesWriter& writer )
{
	std::shared_ptr<CKvBlockNode> gameConfigs = std::make_shared<CKvBlockNode>( "gameConfigs" );

	settings::SaveGameConfigs( m_ConfigManager, *gameConfigs );

	return writer.WriteBlock( *gameConfigs );
}
}