#include <stdexcept>

#include "keyvalues/Keyvalues.h"

#include "CGameConfig.h"
#include "CGameConfigManager.h"

#include "GameConfigIO.h"

namespace settings
{
std::shared_ptr<CGameConfig> LoadGameConfig( const kv::Block& block )
{
	auto name = block.FindFirstChild<kv::KV>( "name" );
	auto basePath = block.FindFirstChild<kv::KV>( "basePath" );
	auto gameDir = block.FindFirstChild<kv::KV>( "gameDir" );
	auto modDir = block.FindFirstChild<kv::KV>( "modDir" );

	if( !name || !basePath || !gameDir || !modDir )
	{
		return nullptr;
	}

	try
	{
		auto newConfig = std::make_shared<settings::CGameConfig>( name->GetValue().CStr() );

		newConfig->SetBasePath( basePath->GetValue().CStr() );
		newConfig->SetGameDir( gameDir->GetValue().CStr() );
		newConfig->SetModDir( modDir->GetValue().CStr() );

		return newConfig;
	}
	catch( const std::invalid_argument& )
	{
		return nullptr;
	}
}

std::pair<size_t, size_t> LoadGameConfigs( const kv::Block& block, std::shared_ptr<CGameConfigManager> manager )
{
	if( !manager )
		return { 0, 0 };

	size_t uiCount = 0;

	size_t uiTotal = 0;

	const auto& children = block.GetChildren();

	for( const auto& child : children )
	{
		if( child->GetKey() != "config" )
			continue;

		if( child->GetType() != kv::NodeType::BLOCK )
			continue;

		++uiTotal;

		auto configBlock = static_cast<kv::Block*>( child );

		auto config = LoadGameConfig( *configBlock );

		if( config )
		{
			if( manager->AddConfig( config ) )
				++uiCount;
		}
	}

	return { uiCount, uiTotal };
}

bool SaveGameConfig( const std::shared_ptr<const CGameConfig>& config, kv::Block& block )
{
	if( !config )
		return false;

	if( !( *config->GetName() ) )
		return false;

	auto pConfigBlock = new kv::Block( "config" );

	pConfigBlock->AddKeyvalue( "name", config->GetName() );
	pConfigBlock->AddKeyvalue( "basePath", config->GetBasePath() );
	pConfigBlock->AddKeyvalue( "gameDir", config->GetGameDir() );
	pConfigBlock->AddKeyvalue( "modDir", config->GetModDir() );

	block.GetChildren().push_back( pConfigBlock );

	return true;
}

size_t SaveGameConfigs( const std::shared_ptr<const CGameConfigManager>& manager, kv::Block& block )
{
	if( !manager )
		return 0;

	size_t uiCount = 0;

	const auto& configs = manager->GetConfigs();

	for( const auto& config : configs )
	{
		if( SaveGameConfig( config, block ) )
			++uiCount;
	}

	return uiCount;
}
}