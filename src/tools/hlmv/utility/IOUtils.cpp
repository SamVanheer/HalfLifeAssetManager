#include <codecvt>
#include <locale>
#include <memory>
#include <string>

#include "keyvalues/Keyvalues.h"

#include "shared/Logging.h"

#include "cvar/CVar.h"
#include "cvar/CVarUtils.h"

#include "utility/Color.h"

#include "IOUtils.h"

bool LoadColorSetting( const kv::Block& settings, const char* const pszName, Color& color, const bool bHasAlpha )
{
	if( !pszName || !( *pszName ) )
		return false;

	if( auto groundColor = settings.FindFirstChild( pszName ) )
	{
		if( groundColor->GetType() == kv::NodeType::KEYVALUE )
		{
			const std::string& sValue = static_cast<kv::KV*>( groundColor )->GetValue();

			if( ParseColor( sValue.c_str(), color, bHasAlpha ) )
			{
				return true;
			}
			else
			{
				Warning( "Setting \"%s\" has invalid syntax! (value: \"%s\")\n", pszName, sValue.c_str() );
			}
		}
		else
		{
			Warning( "Setting \"%s\" is not a keyvalue!\n", pszName );
		}
	}

	return false;
}

bool SaveColorSetting( kv::Writer& writer, const char* const pszName, const Color& color, const bool bHasAlpha )
{
	const auto value = ColorToString(color, bHasAlpha);

	return writer.WriteKeyvalue( pszName, value.c_str());
}

bool LoadColorCVarSetting( const kv::Block& settings, const char* const pszName, const char* const pszCVar, const bool bHasAlpha )
{
	assert( pszCVar );

	Color color;

	if( !LoadColorSetting( settings, pszName, color, bHasAlpha ) )
		return false;

	cvar::CCVar* pLightingR;
	cvar::CCVar* pLightingG;
	cvar::CCVar* pLightingB;

	if( !cvar::GetColorCVars( pszCVar, &pLightingR, &pLightingG, &pLightingB ) )
		return false;

	pLightingR->SetInt( color.GetRed() );
	pLightingG->SetInt( color.GetGreen() );
	pLightingB->SetInt( color.GetBlue() );

	return true;
}

bool SaveColorCVarSetting( kv::Writer& writer, const char* const pszName, const char* const pszCVar, const bool bHasAlpha )
{
	assert( pszCVar );

	cvar::CCVar* pLightingR;
	cvar::CCVar* pLightingG;
	cvar::CCVar* pLightingB;

	if( !cvar::GetColorCVars( pszCVar, &pLightingR, &pLightingG, &pLightingB ) )
		return false;

	Color color( pLightingR->GetInt(), pLightingG->GetInt(), pLightingB->GetInt() );

	return SaveColorSetting( writer, pszName, color );
}

bool LoadArchiveCVars( const kv::Block& cvars )
{
	const auto& children = cvars.GetChildren();

	for( const auto& child : children )
	{
		if( child->GetType() != kv::NodeType::KEYVALUE )
		{
			continue;
		}

		auto kv = static_cast<kv::KV*>( child );

		g_pCVar->SetCVarString( kv->GetKey().c_str(), kv->GetValue().c_str() );
	}

	return true;
}

namespace
{
static void SaveArchiveCVarsCallback( void* pObject, const cvar::CCVar& cvar )
{
	auto& writer = *reinterpret_cast<kv::Writer*>( pObject );

	writer.WriteKeyvalue( cvar.GetName(), cvar.GetString() );
}
}

bool SaveArchiveCVars( kv::Writer& writer, const char* const pszBlockName )
{
	writer.BeginBlock( pszBlockName );

	g_pCVar->ArchiveCVars( &SaveArchiveCVarsCallback, &writer );

	writer.EndBlock();

	return true;
}

FILE* utf8_fopen(const char* filename, const char* mode)
{
#ifdef WIN32
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> convert;

	try
	{
		auto wideFilename = convert.from_bytes(filename);
		auto wideMode = convert.from_bytes(mode);

		return _wfopen(wideFilename.c_str(), wideMode.c_str());
	}
	catch (const std::exception& e)
	{
		auto error = e.what();
	}
#else
	return fopen(filename, mode);
#endif
}
