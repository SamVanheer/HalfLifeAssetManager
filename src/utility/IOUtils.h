#ifndef UTILITY_IOUTILS_H
#define UTILITY_IOUTILS_H

#include "keyvalues/KVForward.h"

class Color;

bool LoadColorSetting( const kv::Block& settings, const char* const pszName, Color& color, const bool bHasAlpha = false );

bool SaveColorSetting( kv::Writer& writer, const char* const pszName, const Color& color, const bool bHasAlpha = false );

bool LoadColorCVarSetting( const kv::Block& settings, const char* const pszName, const char* const pszCVar, const bool bHasAlpha = false );

bool SaveColorCVarSetting( kv::Writer& writer, const char* const pszName, const char* const pszCVar, const bool bHasAlpha = false );

bool LoadArchiveCVars( const kv::Block& cvars );

bool SaveArchiveCVars( kv::Writer& writer, const char* const pszBlockName );

#endif //UTILITY_IOUTILS_H