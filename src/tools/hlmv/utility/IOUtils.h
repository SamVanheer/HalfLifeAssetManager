#ifndef UTILITY_IOUTILS_H
#define UTILITY_IOUTILS_H

#include <cstdio>

#include "keyvalues/KVForward.h"

class Color;

bool LoadColorSetting( const kv::Block& settings, const char* const pszName, Color& color, const bool bHasAlpha = false );

bool SaveColorSetting( kv::Writer& writer, const char* const pszName, const Color& color, const bool bHasAlpha = false );

bool LoadColorCVarSetting( const kv::Block& settings, const char* const pszName, const char* const pszCVar, const bool bHasAlpha = false );

bool SaveColorCVarSetting( kv::Writer& writer, const char* const pszName, const char* const pszCVar, const bool bHasAlpha = false );

bool LoadArchiveCVars( const kv::Block& cvars );

bool SaveArchiveCVars( kv::Writer& writer, const char* const pszBlockName );

/**
*	@brief Wrapper around fopen to open filenames containing UTF8 characters on Windows
*/
FILE* utf8_fopen(const char* filename, const char* mode);

#endif //UTILITY_IOUTILS_H