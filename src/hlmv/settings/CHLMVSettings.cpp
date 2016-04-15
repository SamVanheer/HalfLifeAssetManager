#include "common/Logging.h"

#include "keyvalues/Keyvalues.h"

#include "utility/IOUtils.h"

#include "settings/CGameConfig.h"
#include "settings/CGameConfigManager.h"

#include "CHLMVSettings.h"

namespace hlmv
{
const Color CHLMVSettings::DEFAULT_GROUND_COLOR = Color( 216, 216, 175 );

const Color CHLMVSettings::DEFAULT_BACKGROUND_COLOR = Color( 63, 127, 127 );

const Color CHLMVSettings::DEFAULT_CROSSHAIR_COLOR = Color( 255, 0, 0 );

const Color CHLMVSettings::DEFAULT_LIGHT_COLOR = Color( 255, 255, 255 );

const Color CHLMVSettings::DEFAULT_WIREFRAME_COLOR = Color( 255, 0, 0 );

CHLMVSettings::CHLMVSettings()
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
	m_LightColor		= other.m_LightColor;
	m_WireframeColor	= other.m_WireframeColor;
}

bool CHLMVSettings::LoadFromFile( const std::shared_ptr<CKvBlockNode>& root )
{
	if( !CBaseSettings::LoadFromFile( root ) )
		return false;

	auto settings = root->FindFirstChild<CKvBlockNode>( "hlmvSettings" );

	if( settings )
	{
		auto active = settings->FindFirstChild<CKeyvalue>( "activeConfig" );

		if( active )
		{
			GetConfigManager()->SetActiveConfig( active->GetValue().CStr() );
		}

		if( auto block = settings->FindFirstChild<CKvBlockNode>( "recentFiles" ) )
		{
			const auto& children = block->GetChildren();

			for( const auto& child : children )
			{
				if( child->GetType() != KVNode_Keyvalue )
					continue;

				if( child->GetKey() != "recentFile" )
					continue;

				auto file = std::static_pointer_cast<CKeyvalue>( child );

				m_RecentFiles->Add( file->GetValue().CStr() );
			}
		}

		LoadColorSetting( settings, "groundColor", m_GroundColor );
		LoadColorSetting( settings, "backgroundColor", m_BackgroundColor );
		LoadColorSetting( settings, "crosshairColor", m_CrosshairColor );
		LoadColorSetting( settings, "lightColor", m_LightColor );
		LoadColorSetting( settings, "wireframeColor", m_WireframeColor );
	}

	return true;
}

bool CHLMVSettings::SaveToFile( CKeyvaluesWriter& writer )
{
	if( !CBaseSettings::SaveToFile( writer ) )
		return false;

	writer.BeginBlock( "hlmvSettings" );

	if( auto activeConfig = GetConfigManager()->GetActiveConfig() )
	{
		writer.WriteKeyvalue( "activeConfig", activeConfig->GetName() );
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
	SaveColorSetting( writer, "lightColor", m_LightColor );
	SaveColorSetting( writer, "wireframeColor", m_WireframeColor );

	writer.EndBlock();

	return true;
}
}