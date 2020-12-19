#ifndef ENGINE_SHARED_SPRITE_CSPRITE_H
#define ENGINE_SHARED_SPRITE_CSPRITE_H

#include "sprite.hpp"

namespace sprite
{
bool LoadSprite( const char* const pszFilename, msprite_t*& pSprite );

void FreeSprite( msprite_t* pSprite );
}

#endif //ENGINE_SHARED_SPRITE_CSPRITE_H