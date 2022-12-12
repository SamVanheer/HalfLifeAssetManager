#pragma once

#include "formats/sprite/SpriteFileFormat.hpp"

class QOpenGLFunctions_1_1;

namespace sprite
{
bool LoadSprite(QOpenGLFunctions_1_1* openglFunctions, const char* const pszFilename, msprite_t*& pSprite );

void FreeSprite(QOpenGLFunctions_1_1* openglFunctions, msprite_t* pSprite );
}
