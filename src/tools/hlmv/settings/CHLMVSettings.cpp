#include "shared/Logging.h"

#include "keyvalues/Keyvalues.h"

#include "utility/IOUtils.h"

#include "settings/CGameConfig.h"
#include "settings/CCmdLineConfig.h"
#include "settings/ConfigIO.h"

#include "cvar/CVar.h"

#include "CHLMVSettings.h"

#include <cmath>

namespace hlmv
{
const size_t CHLMVSettings::MAX_RECENT_FILES = 4;

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

	m_UseTimerForFrame = other.m_UseTimerForFrame;

	m_InvertHorizontalDraggingDirection = other.m_InvertHorizontalDraggingDirection;
	m_InvertVerticalDraggingDirection = other.m_InvertVerticalDraggingDirection;

	m_GroundColor		= other.m_GroundColor;
	m_BackgroundColor	= other.m_BackgroundColor;
	m_CrosshairColor	= other.m_CrosshairColor;

	m_flFloorLength		= other.m_flFloorLength;

	m_szStudioMdl		= other.m_szStudioMdl;
	m_szMdlDec			= other.m_szMdlDec;

	*m_StudioMdlConfigs = *other.m_StudioMdlConfigs;
	*m_MdlDecConfigs	= *other.m_MdlDecConfigs;

	m_szDefaultOutputFileDir = other.m_szDefaultOutputFileDir;

	m_IsWindowMaximized = other.m_IsWindowMaximized;
	m_WindowWidth = other.m_WindowWidth;
	m_WindowHeight = other.m_WindowHeight;

	m_CurrentCameraName = other.m_CurrentCameraName;
	m_CurrentControlPanelName = other.m_CurrentControlPanelName;

	m_CorrectSequenceGroupFileNames = other.m_CorrectSequenceGroupFileNames;
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
	m_flFloorLength = clamp( static_cast<float>( fabs( flLength ) ), MIN_FLOOR_LENGTH, MAX_FLOOR_LENGTH );
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
			GetConfigManager()->SetActiveConfig( active->GetValue().c_str() );
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

				m_RecentFiles->Add(file->GetValue());
			}
		}

		if (auto useTimerForFrame = settings->FindFirstChild<kv::KV>("useTimerForFrame"); useTimerForFrame)
		{
			SetUseTimerForFrame(useTimerForFrame->GetValue() == "true");
		}

		if (auto invertHorizontalDragging = settings->FindFirstChild<kv::KV>("invertHorizontalDraggingDirection"))
		{
			SetInvertHoritonzalDraggingDirection(invertHorizontalDragging->GetValue() == "true");
		}

		if (auto invertVerticalDragging = settings->FindFirstChild<kv::KV>("invertVerticalDraggingDirection"))
		{
			SetInvertVerticalDraggingDirection(invertVerticalDragging->GetValue() == "true");
		}

		if (auto kv = settings->FindFirstChild<kv::KV>("correctSequenceGroupFileNames"); kv)
		{
			m_CorrectSequenceGroupFileNames = kv->GetValue() == "true";
		}

		LoadColorSetting( *settings, "groundColor", m_GroundColor );
		LoadColorSetting( *settings, "backgroundColor", m_BackgroundColor );
		LoadColorSetting( *settings, "crosshairColor", m_CrosshairColor );

		if( auto floor = settings->FindFirstChild<kv::KV>( "floorLength" ) )
		{
			SetFloorLength(static_cast<float>(std::stod(floor->GetValue())));
		}

		if( auto studiomdl = settings->FindFirstChild<kv::KV>( "studiomdl" ) )
		{
			m_szStudioMdl = studiomdl->GetValue();
		}

		if( auto mdldec = settings->FindFirstChild<kv::KV>( "mdldec" ) )
		{
			m_szMdlDec = mdldec->GetValue();
		}

		if( auto cmdLineSettingsList = settings->FindFirstChild<kv::Block>( "StudioMdlConfigs" ) )
		{
			settings::LoadGameConfigs( *cmdLineSettingsList, m_StudioMdlConfigs, settings::LoadCmdLineConfig, settings::CCmdLineConfig::IO_BLOCK_NAME );
		}

		if( auto cmdLineSettingsList = settings->FindFirstChild<kv::Block>( "MdlDecConfigs" ) )
		{
			settings::LoadGameConfigs( *cmdLineSettingsList, m_MdlDecConfigs, settings::LoadCmdLineConfig, settings::CCmdLineConfig::IO_BLOCK_NAME );
		}

		if( auto activeConfig = settings->FindFirstChild<kv::KV>( "activeStudioMdlConfig" ) )
		{
			m_StudioMdlConfigs->SetActiveConfig(activeConfig->GetValue().c_str());
		}

		if( auto activeConfig = settings->FindFirstChild<kv::KV>( "activeMdlDecConfig" ) )
		{
			m_MdlDecConfigs->SetActiveConfig(activeConfig->GetValue().c_str());
		}

		if( auto outputdir = settings->FindFirstChild<kv::KV>( "defaultOutputFileDir" ) )
		{
			m_szDefaultOutputFileDir = outputdir->GetValue();
		}

		if (auto block = settings->FindFirstChild<kv::Block>("window"); block)
		{
			if (auto maximized = block->FindFirstChild<kv::KV>("maximized"); maximized)
			{
				m_IsWindowMaximized = maximized->GetValue() == "true";
			}

			if (auto x = block->FindFirstChild<kv::KV>("x"); x)
			{
				m_WindowX = std::stoi(x->GetValue());
			}

			if (auto y = block->FindFirstChild<kv::KV>("y"); y)
			{
				m_WindowY = std::stoi(y->GetValue());
			}

			if (auto width = block->FindFirstChild<kv::KV>("width"); width)
			{
				m_WindowWidth = std::stoi(width->GetValue());
			}

			if (auto height = block->FindFirstChild<kv::KV>("height"); height)
			{
				m_WindowHeight = std::stoi(height->GetValue());
			}
		}

		if (auto block = settings->FindFirstChild<kv::Block>("controlPanelState"); block)
		{
			if (auto kv = block->FindFirstChild<kv::KV>("cameraName"); kv)
			{
				m_CurrentCameraName = kv->GetValue();
			}

			if (auto kv = block->FindFirstChild<kv::KV>("controlPanelName"); kv)
			{
				m_CurrentControlPanelName = kv->GetValue();
			}
		}
	}

	return true;
}

bool CHLMVSettings::SaveToFile( kv::Writer& writer )
{
	if( !CBaseSettings::SaveToFile( writer ) )
		return false;

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

	writer.WriteKeyvalue("useTimerForFrame", m_UseTimerForFrame ? "true" : "false");
	writer.WriteKeyvalue("invertHorizontalDraggingDirection", m_InvertHorizontalDraggingDirection ? "true" : "false");
	writer.WriteKeyvalue("invertVerticalDraggingDirection", m_InvertVerticalDraggingDirection ? "true" : "false");
	writer.WriteKeyvalue("correctSequenceGroupFileNames", m_CorrectSequenceGroupFileNames ? "true" : "false");

	SaveColorSetting( writer, "groundColor", m_GroundColor );
	SaveColorSetting( writer, "backgroundColor", m_BackgroundColor );
	SaveColorSetting( writer, "crosshairColor", m_CrosshairColor );

	auto floorLength = std::to_string(GetFloorLength());

	writer.WriteKeyvalue( "floorLength", floorLength.c_str());

	writer.WriteKeyvalue( "studiomdl", m_szStudioMdl.c_str() );
	writer.WriteKeyvalue( "mdldec", m_szMdlDec.c_str() );

	writer.BeginBlock( "StudioMdlConfigs" );

	settings::SaveGameConfigs( std::static_pointer_cast<const settings::CCmdLineConfigManager>( m_StudioMdlConfigs ), writer, settings::SaveCmdLineConfig );

	writer.EndBlock();

	writer.BeginBlock( "MdlDecConfigs" );

	settings::SaveGameConfigs( std::static_pointer_cast<const settings::CCmdLineConfigManager>( m_MdlDecConfigs ), writer, settings::SaveCmdLineConfig );

	writer.EndBlock();

	if( auto config = m_StudioMdlConfigs->GetActiveConfig() )
	{
		writer.WriteKeyvalue( "activeStudioMdlConfig", config->GetName().c_str() );
	}

	if( auto config = m_MdlDecConfigs->GetActiveConfig() )
	{
		writer.WriteKeyvalue( "activeMdlDecConfig", config->GetName().c_str() );
	}

	writer.WriteKeyvalue( "defaultOutputFileDir", m_szDefaultOutputFileDir.c_str() );

	writer.BeginBlock("window");

	writer.WriteKeyvalue("maximized", m_IsWindowMaximized ? "true" : "false");
	writer.WriteKeyvalue("x", std::to_string(m_WindowX).c_str());
	writer.WriteKeyvalue("y", std::to_string(m_WindowY).c_str());
	writer.WriteKeyvalue("width", std::to_string(m_WindowWidth).c_str());
	writer.WriteKeyvalue("height", std::to_string(m_WindowHeight).c_str());

	writer.EndBlock();

	writer.BeginBlock("controlPanelState");

	writer.WriteKeyvalue("cameraName", m_CurrentCameraName.c_str());
	writer.WriteKeyvalue("controlPanelName", m_CurrentControlPanelName.c_str());

	writer.EndBlock();

	writer.EndBlock();

	return !writer.ErrorOccurred();
}
}
