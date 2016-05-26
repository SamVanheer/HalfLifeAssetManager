#include <stdexcept>

#include "keyvalues/Keyvalues.h"

#include "CGameConfig.h"

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

bool SaveGameConfig( const CGameConfig& config, kv::Block& block )
{
	if( !( *config.GetName() ) )
		return false;

	auto pConfigBlock = new kv::Block( "config" );

	pConfigBlock->AddKeyvalue( "name", config.GetName() );
	pConfigBlock->AddKeyvalue( "basePath", config.GetBasePath() );
	pConfigBlock->AddKeyvalue( "gameDir", config.GetGameDir() );
	pConfigBlock->AddKeyvalue( "modDir", config.GetModDir() );

	block.GetChildren().push_back( pConfigBlock );

	return true;
}
}