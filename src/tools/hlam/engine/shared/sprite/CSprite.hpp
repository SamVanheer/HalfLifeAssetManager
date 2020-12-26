#pragma once

#include "engine/shared/sprite/SpriteFileFormat.hpp"

namespace sprite
{
bool LoadSprite( const char* const pszFilename, msprite_t*& pSprite );

void FreeSprite( msprite_t* pSprite );
}
