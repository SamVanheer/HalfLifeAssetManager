#ifndef SETTINGS_GAMECONFIGIO_H
#define SETTINGS_GAMECONFIGIO_H

#include <memory>

#include "keyvalues/KVForward.h"

namespace settings
{
class CGameConfig;
class CGameConfigManager;

/**
*	Loads a single game configuration from a block.
*	@param block keyvalue block that contains a game configuration.
*	@return Game configuration, or nullptr if the configuration was ill-formed.
*/
std::shared_ptr<CGameConfig> LoadGameConfig( const kv::Block& block );

/**
*	Loads a set of game configurations from a block.
*	@param block keyvalue block that contains a list of game configurations.
*	@param manager Manager to add configurations to.
*	@return pair containing the number of configurations that were loaded in, and the total number of configurations encountered in the block. Ill-formed configurations are ignored.
*/
std::pair<size_t, size_t> LoadGameConfigs( const kv::Block& block, std::shared_ptr<CGameConfigManager> manager );

/**
*	Saves a single game configuration to a block. The configuration is added as a child block.
*	@param config Configuration to save.
*	@param block Block to add the child block to.
*	@return true on success, false otherwise.
*/
bool SaveGameConfig( const std::shared_ptr<const CGameConfig>& config, kv::Block& block );

/**
*	Saves all of the game configurations that the given manager has.
*	They are added as child blocks to the given block.
*	@param manager Manager to get configurations from.
*	@param block Block to add the child blocks to.
*	@return Number of configurations that were saved.
*/
size_t SaveGameConfigs( const std::shared_ptr<const CGameConfigManager>& manager, kv::Block& block );
}

#endif //SETTINGS_GAMECONFIGIO_H