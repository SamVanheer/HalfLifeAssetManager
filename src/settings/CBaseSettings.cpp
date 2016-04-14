#include <cassert>

#include "common/Logging.h"

#include "CGameConfig.h"
#include "CGameConfigManager.h"

#include "GameConfigIO.h"

#include "keyvalues/CKeyvalueNode.h"
#include "keyvalues/CKeyvalue.h"
#include "keyvalues/CKvBlockNode.h"
#include "keyvalues/CKeyvalues.h"
#include "keyvalues/CKeyvaluesParser.h"
#include "keyvalues/CKeyvaluesWriter.h"

#include "filesystem/CFileSystem.h"

#include "utility/CString.h"

#include "CBaseSettings.h"

namespace settings
{
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
	}

	//Initialize file system based on settings.
	if( bResult )
	{
		bResult = InitializeFileSystem();
	}

	if( bResult )
	{
		bResult = PostInitialize( pszFilename );
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

	//TODO: put this somewhere else
	const char* pszDirs[] =
	{
		"",
		"_downloads",
		"_addon",
		"_hd"
	};

	//Note: do not use a reference here. Varargs doesn't convert it, so it'll end up being const char**.

	//Add mod dirs first, since they override game dirs.
	if( strcmp( config->GetGameDir(), config->GetModDir() ) )
	{
		for( const auto pszDir : pszDirs )
		{
			szPath.Format( "%s%s", config->GetModDir(), pszDir );

			fileSystem().AddSearchPath( szPath.CStr() );
		}
	}

	for( const auto pszDir : pszDirs )
	{
		szPath.Format( "%s%s", config->GetGameDir(), pszDir );

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

	return LoadFromFile( parser.GetKeyvalues()->GetRoot() );
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

bool CBaseSettings::LoadFromFile( const std::shared_ptr<CKvBlockNode>& root )
{
	auto configs = root->FindFirstChild<CKvBlockNode>( "gameConfigs" );

	if( configs )
	{
		const auto result = settings::LoadGameConfigs( *configs, m_ConfigManager );

		if( result.first < result.second )
			Warning( "%u game configurations failed to load\n", result.second - result.first );
	}

	return true;
}

bool CBaseSettings::SaveToFile( CKeyvaluesWriter& writer )
{
	std::shared_ptr<CKvBlockNode> gameConfigs = std::make_shared<CKvBlockNode>( "gameConfigs" );

	settings::SaveGameConfigs( m_ConfigManager, *gameConfigs );

	writer.WriteBlock( *gameConfigs );

	return true;
}
}