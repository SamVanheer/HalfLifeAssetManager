#include <stdexcept>

#include "keyvalues/CKeyvalueNode.h"
#include "keyvalues/CKeyvalue.h"
#include "keyvalues/CKvBlockNode.h"

#include "CGameConfig.h"
#include "CGameConfigManager.h"

#include "GameConfigIO.h"

namespace settings
{
std::shared_ptr<CGameConfig> LoadGameConfig( const CKvBlockNode& block )
{
	auto name = block.FindFirstChild<CKeyvalue>( "name" );
	auto basePath = block.FindFirstChild<CKeyvalue>( "basePath" );
	auto gameDir = block.FindFirstChild<CKeyvalue>( "gameDir" );
	auto modDir = block.FindFirstChild<CKeyvalue>( "modDir" );

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

std::pair<size_t, size_t> LoadGameConfigs( const CKvBlockNode& block, std::shared_ptr<CGameConfigManager> manager )
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

		if( child->GetType() != KVNode_Block )
			continue;

		++uiTotal;

		auto block = std::static_pointer_cast<CKvBlockNode>( child );

		auto config = LoadGameConfig( *block );

		if( config )
		{
			if( manager->AddConfig( config ) )
				++uiCount;
		}
	}

	return { uiCount, uiTotal };
}

bool SaveGameConfig( const std::shared_ptr<const CGameConfig>& config, CKvBlockNode& block )
{
	if( !config )
		return false;

	if( !( *config->GetName() ) )
		return false;

	std::shared_ptr<CKvBlockNode> configBlock = std::make_shared<CKvBlockNode>( "config" );

	configBlock->AddKeyvalue( "name", config->GetName() );
	configBlock->AddKeyvalue( "basePath", config->GetBasePath() );
	configBlock->AddKeyvalue( "gameDir", config->GetGameDir() );
	configBlock->AddKeyvalue( "modDir", config->GetModDir() );

	block.GetChildren().push_back( configBlock );

	return true;
}

size_t SaveGameConfigs( const std::shared_ptr<const CGameConfigManager>& manager, CKvBlockNode& block )
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