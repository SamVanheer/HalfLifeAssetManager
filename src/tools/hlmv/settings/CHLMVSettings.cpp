#include "shared/Logging.h"

#include "keyvalues/Keyvalues.h"

#include "utility/IOUtils.h"

#include "settings/CGameConfig.h"
#include "settings/CGameConfigManager.h"

#include "cvar/CVar.h"
#include "cvar/CCVarSystem.h"

#include "CHLMVSettings.h"

namespace hlmv
{
const Color CHLMVSettings::DEFAULT_GROUND_COLOR = Color( 216, 216, 175 );

const Color CHLMVSettings::DEFAULT_BACKGROUND_COLOR = Color( 63, 127, 127 );

const Color CHLMVSettings::DEFAULT_CROSSHAIR_COLOR = Color( 255, 0, 0 );

const Color CHLMVSettings::DEFAULT_LIGHT_COLOR = Color( 255, 255, 255 );

const Color CHLMVSettings::DEFAULT_WIREFRAME_COLOR = Color( 255, 0, 0 );

const float CHLMVSettings::MIN_FLOOR_LENGTH = 0;

const float CHLMVSettings::MAX_FLOOR_LENGTH = 2048;

const float CHLMVSettings::DEFAULT_FLOOR_LENGTH = 100;

CHLMVSettings::CHLMVSettings( filesystem::IFileSystem* const pFileSystem )
	: CBaseSettings( pFileSystem )
{
}

CHLMVSettings::~CHLMVSettings()
{
}

CHLMVSettings::CHLMVSettings( const CHLMVSettings& other )
	: CBaseSettings( other )
{
	Copy( other );
}

CHLMVSettings& CHLMVSettings::operator=( const CHLMVSettings& other )
{
	CBaseSettings::operator=( other );

	if( this != &other )
	{
		Copy( other );
	}

	return *this;
}

void CHLMVSettings::Copy( const CHLMVSettings& other )
{
	*m_RecentFiles		= *other.m_RecentFiles;

	m_GroundColor		= other.m_GroundColor;
	m_BackgroundColor	= other.m_BackgroundColor;
	m_CrosshairColor	= other.m_CrosshairColor;

	m_flFloorLength		= other.m_flFloorLength;

	m_szStudioMdl		= other.m_szStudioMdl;
	m_szMdlDec			= other.m_szMdlDec;
}

void CHLMVSettings::ActiveConfigChanged( const std::shared_ptr<settings::CGameConfig>& oldConfig, const std::shared_ptr<settings::CGameConfig>& newConfig )
{
	//Active config changed, reinit the filesystem
	if( !InitializeFileSystem() )
	{
		Error( "Error reinitializing the filesystem after configuration change from \"%s\" to \"%s\"\n", oldConfig ? oldConfig->GetName() : "None", newConfig ? newConfig->GetName() : "None" );
	}
}

void CHLMVSettings::SetFloorLength( float flLength )
{
	m_flFloorLength = clamp( abs( flLength ), MIN_FLOOR_LENGTH, MAX_FLOOR_LENGTH );
}

bool CHLMVSettings::PostInitialize( const char* const pszFilename )
{
	GetConfigManager()->SetListener( this );

	return true;
}

void CHLMVSettings::PreShutdown( const char* const pszFilename )
{
	GetConfigManager()->SetListener( nullptr );
}

bool CHLMVSettings::LoadFromFile( const kv::Block& root )
{
	if( !CBaseSettings::LoadFromFile( root ) )
		return false;

	auto settings = root.FindFirstChild<kv::Block>( "hlmvSettings" );

	if( settings )
	{
		auto active = settings->FindFirstChild<kv::KV>( "activeConfig" );

		if( active )
		{
			GetConfigManager()->SetActiveConfig( active->GetValue().CStr() );
		}

		if( auto block = settings->FindFirstChild<kv::Block>( "recentFiles" ) )
		{
			const auto& children = block->GetChildren();

			for( auto it = children.rbegin(), end = children.rend(); it != end; ++it )
			{
				const auto& child = *it;

				if( child->GetType() != kv::NodeType::KEYVALUE )
					continue;

				if( child->GetKey() != "recentFile" )
					continue;

				auto file = static_cast<kv::KV*>( child );

				m_RecentFiles->Add( file->GetValue().CStr() );
			}
		}

		LoadColorSetting( *settings, "groundColor", m_GroundColor );
		LoadColorSetting( *settings, "backgroundColor", m_BackgroundColor );
		LoadColorSetting( *settings, "crosshairColor", m_CrosshairColor );

		if( auto floor = settings->FindFirstChild<kv::KV>( "floorLength" ) )
		{
			SetFloorLength( static_cast<float>( strtod( floor->GetValue().CStr(), nullptr ) ) );
		}

		if( auto studiomdl = settings->FindFirstChild<kv::KV>( "studiomdl" ) )
		{
			m_szStudioMdl = studiomdl->GetValue();
		}

		if( auto mdldec = settings->FindFirstChild<kv::KV>( "mdldec" ) )
		{
			m_szMdlDec = mdldec->GetValue();
		}
	}

	return true;
}

bool CHLMVSettings::SaveToFile( kv::Writer& writer )
{
	if( !CBaseSettings::SaveToFile( writer ) )
		return false;

	char szBuffer[ MAX_BUFFER_LENGTH ];

	writer.BeginBlock( "hlmvSettings" );

	if( auto activeConfig = GetConfigManager()->GetActiveConfig() )
	{
		writer.WriteKeyvalue( "activeConfig", activeConfig->GetName() );
	}
	else
	{
		writer.WriteComment( "No active config" );
	}

	writer.BeginBlock( "recentFiles" );

	for( const auto& file : m_RecentFiles->GetFiles() )
	{
		writer.WriteKeyvalue( "recentFile", file.c_str() );
	}

	writer.EndBlock();

	SaveColorSetting( writer, "groundColor", m_GroundColor );
	SaveColorSetting( writer, "backgroundColor", m_BackgroundColor );
	SaveColorSetting( writer, "crosshairColor", m_CrosshairColor );

	if( !PrintfSuccess( snprintf( szBuffer, sizeof( szBuffer ), "%f", GetFloorLength() ), sizeof( szBuffer ) ) )
		return false;

	writer.WriteKeyvalue( "floorLength", szBuffer );

	writer.WriteKeyvalue( "studiomdl", m_szStudioMdl.CStr() );
	writer.WriteKeyvalue( "mdldec", m_szMdlDec.CStr() );

	writer.EndBlock();

	return !writer.ErrorOccurred();
}
}