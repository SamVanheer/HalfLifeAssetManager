#include "keyvalues/Keyvalues.h"

#include "settings/CGameConfig.h"
#include "settings/CGameConfigManager.h"

#include "CHLMVSettings.h"

namespace hlmv
{
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
	*m_RecentFiles = *other.m_RecentFiles;
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

	writer.EndBlock();

	return true;
}
}