#ifndef UTILITY_IOUTILS_H
#define UTILITY_IOUTILS_H

#include <memory>

class CKvBlockNode;
class CKeyvaluesWriter;
class Color;

bool LoadColorSetting( const std::shared_ptr<CKvBlockNode>& settings, const char* const pszName, Color& color, const bool bHasAlpha = false );

bool SaveColorSetting( CKeyvaluesWriter& writer, const char* const pszName, const Color& color, const bool bHasAlpha = false );

#endif //UTILITY_IOUTILS_H