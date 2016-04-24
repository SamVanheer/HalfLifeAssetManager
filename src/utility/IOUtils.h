#ifndef UTILITY_IOUTILS_H
#define UTILITY_IOUTILS_H

class CKvBlockNode;
class CKeyvaluesWriter;
class Color;

bool LoadColorSetting( const CKvBlockNode& settings, const char* const pszName, Color& color, const bool bHasAlpha = false );

bool SaveColorSetting( CKeyvaluesWriter& writer, const char* const pszName, const Color& color, const bool bHasAlpha = false );

bool LoadColorCVarSetting( const CKvBlockNode& settings, const char* const pszName, const char* const pszCVar, const bool bHasAlpha = false );

bool SaveColorCVarSetting( CKeyvaluesWriter& writer, const char* const pszName, const char* const pszCVar, const bool bHasAlpha = false );

bool LoadArchiveCVars( const CKvBlockNode& cvars );

bool SaveArchiveCVars( CKeyvaluesWriter& writer, const char* const pszBlockName );

#endif //UTILITY_IOUTILS_H