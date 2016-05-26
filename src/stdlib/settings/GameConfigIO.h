#ifndef SETTINGS_GAMECONFIGIO_H
#define SETTINGS_GAMECONFIGIO_H

#include <memory>

#include "keyvalues/KVForward.h"

namespace settings
{
class CGameConfig;

/**
*	Loads a single game configuration from a block.
*	@param block keyvalue block that contains a game configuration.
*	@return Game configuration, or nullptr if the configuration was ill-formed.
*/
std::shared_ptr<CGameConfig> LoadGameConfig( const kv::Block& block );

/**
*	Saves a single game configuration to a block. The configuration is added as a child block.
*	@param config Configuration to save.
*	@param block Block to add the child block to.
*	@return true on success, false otherwise.
*/
bool SaveGameConfig( const CGameConfig& config, kv::Block& block );
}

#endif //SETTINGS_GAMECONFIGIO_H