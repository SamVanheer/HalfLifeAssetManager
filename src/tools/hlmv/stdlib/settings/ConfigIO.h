#ifndef SETTINGS_CONFIGIO_H
#define SETTINGS_CONFIGIO_H

#include <cassert>
#include <utility>

#include "keyvalues/Keyvalues.h"

#include "CBaseConfigManager.h"

namespace settings
{
/**
*	Loads a set of game configurations from a block.
*	@param block keyvalue block that contains a list of game configurations.
*	@param manager Manager to add configurations to.
*	@param loadFn Function used to load configs.
*	@return pair containing the number of configurations that were loaded in, and the total number of configurations encountered in the block. Ill-formed configurations are ignored.
*	@tparam CONFIG Config class type.
*	@tparam LOADFN Type of the function used to load configs.
*/
template<typename CONFIG, typename LOADFN>
std::pair<size_t, size_t> LoadGameConfigs( const kv::Block& block, std::shared_ptr<CBaseConfigManager<CONFIG>> manager, LOADFN loadFn, const char* const pszConfigBlockName = "config" )
{
	assert( pszConfigBlockName );

	if( !manager )
		return { 0, 0 };

	size_t uiCount = 0;

	size_t uiTotal = 0;

	const auto& children = block.GetChildren();

	for( const auto& child : children )
	{
		if( child->GetKey() != pszConfigBlockName )
			continue;

		if( child->GetType() != kv::NodeType::BLOCK )
			continue;

		++uiTotal;

		auto configBlock = static_cast<kv::Block*>( child );

		auto config = loadFn( *configBlock );

		if( config )
		{
			if( manager->AddConfig( config ) )
				++uiCount;
		}
	}

	return { uiCount, uiTotal };
}

/**
*	Saves all of the game configurations that the given manager has.
*	They are added as child blocks to the given block.
*	@param manager Manager to get configurations from.
*	@param data Data to pass to the save function.
*	@param saveFn Function used to save configs.
*	@return Number of configurations that were saved.
*	@tparam CONFIG Config class type.
*	@tparam SAVEFN Type of the function used to save configs.
*/
template<typename CONFIG, typename DATA, typename SAVEFN>
size_t SaveGameConfigs( const std::shared_ptr<const CBaseConfigManager<CONFIG>>& manager, DATA& data, SAVEFN saveFn )
{
	if( !manager )
		return 0;

	size_t uiCount = 0;

	const auto& configs = manager->GetConfigs();

	for( const auto& config : configs )
	{
		if( saveFn( *config, data ) )
			++uiCount;
	}

	return uiCount;
}
}

#endif //SETTINGS_CONFIGIO_H