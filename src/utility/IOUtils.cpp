#include "keyvalues/Keyvalues.h"

#include "common/Logging.h"

#include "Color.h"

#include "IOUtils.h"

bool LoadColorSetting(const std::shared_ptr<CKvBlockNode>& settings, const char* const pszName, Color& color, const bool bHasAlpha )
{
	if( !pszName || !( *pszName ) )
		return false;

	if( auto groundColor = settings->FindFirstChild( pszName ) )
	{
		if( groundColor->GetType() == KVNode_Keyvalue )
		{
			const CString& sValue = std::static_pointer_cast<CKeyvalue>( groundColor )->GetValue();

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

bool SaveColorSetting( CKeyvaluesWriter& writer, const char* const pszName, const Color& color, const bool bHasAlpha )
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