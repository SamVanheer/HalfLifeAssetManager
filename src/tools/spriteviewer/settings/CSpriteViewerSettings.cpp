#include "shared/Logging.h"

#include "keyvalues/Keyvalues.h"

#include "utility/IOUtils.h"

#include "CSpriteViewerSettings.h"

namespace sprview
{
const size_t CSpriteViewerSettings::MAX_RECENT_FILES = 4;

const Color CSpriteViewerSettings::DEFAULT_BACKGROUND_COLOR = Color( 63, 127, 127 );

CSpriteViewerSettings::CSpriteViewerSettings( filesystem::IFileSystem* const pFileSystem )
	: CBaseSettings( pFileSystem )
{
}

CSpriteViewerSettings::~CSpriteViewerSettings()
{
}

CSpriteViewerSettings::CSpriteViewerSettings( const CSpriteViewerSettings& other )
	: CBaseSettings( other )
{
	Copy( other );
}

CSpriteViewerSettings& CSpriteViewerSettings::operator=( const CSpriteViewerSettings& other )
{
	CBaseSettings::operator=( other );

	if( this != &other )
	{
		Copy( other );
	}

	return *this;
}

void CSpriteViewerSettings::Copy( const CSpriteViewerSettings& other )
{
	*m_RecentFiles = *other.m_RecentFiles;

	m_BackgroundColor = other.m_BackgroundColor;
}

bool CSpriteViewerSettings::PostInitialize( const char* const pszFilename )
{
	return true;
}

void CSpriteViewerSettings::PreShutdown( const char* const pszFilename )
{
}

bool CSpriteViewerSettings::LoadFromFile( const kv::Block& root )
{
	if( !CBaseSettings::LoadFromFile( root ) )
		return false;

	auto settings = root.FindFirstChild<kv::Block>( "spriteViewerSettings" );

	if( settings )
	{
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

		LoadColorSetting( *settings, "backgroundColor", m_BackgroundColor );
	}

	return true;
}

bool CSpriteViewerSettings::SaveToFile( kv::Writer& writer )
{
	if( !CBaseSettings::SaveToFile( writer ) )
		return false;

	writer.BeginBlock( "spriteViewerSettings" );

	writer.BeginBlock( "recentFiles" );

	for( const auto& file : m_RecentFiles->GetFiles() )
	{
		writer.WriteKeyvalue( "recentFile", file.c_str() );
	}

	writer.EndBlock();

	SaveColorSetting( writer, "backgroundColor", m_BackgroundColor );

	writer.EndBlock();

	return !writer.ErrorOccurred();
}
}