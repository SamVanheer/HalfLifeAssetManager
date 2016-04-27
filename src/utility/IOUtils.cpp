#include <memory>

#include "keyvalues/Keyvalues.h"

#include "shared/Logging.h"

#include "cvar/CVar.h"
#include "cvar/CCVarSystem.h"
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
			const CString& sValue = static_cast<kv::KV*>( groundColor )->GetValue();

			if( ParseColor( sValue.CStr(), color, bHasAlpha ) )
			{
				return true;
			}
			else
			{
				Warning( "Setting \"%s\" has invalid syntax! (value: \"%s\")\n", pszName, sValue.CStr() );
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
	char szBuffer[ MAX_BUFFER_LENGTH ];

	if( ColorToString( color, szBuffer, sizeof( szBuffer ), bHasAlpha ) )
	{
		return writer.WriteKeyvalue( pszName, szBuffer );
	}
	else
	{
		Warning( "Failed to convert setting \"%s\" to string!\n", pszName );
	}

	return false;
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

		cvar::cvars().SetCVarString( kv->GetKey().CStr(), kv->GetValue().CStr() );
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

	cvar::cvars().ArchiveCVars( &SaveArchiveCVarsCallback, &writer );

	writer.EndBlock();

	return true;
}