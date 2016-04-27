#include <cstdio>
#include <limits>

#include "shared/Platform.h"
#include "shared/Logging.h"

#include "CCVarSystem.h"

#include "CCVar.h"

namespace cvar
{
namespace
{
template<typename T>
static char* CreateStringValue( const char* const pszFormat, const T value )
{
	assert( pszFormat );

	//Get buffer size.
	size_t uiSize = static_cast<size_t>( snprintf( nullptr, 0, pszFormat, value ) );

	//Avoid overflow.
	if( uiSize != SIZE_MAX )
		uiSize += 1;

	char* pszValue = new char[ uiSize ];

	const int iRet = snprintf( pszValue, uiSize, pszFormat, value );

	//Fallback
	if( iRet < 0 )
	{
		pszValue[ 0 ] = '\0';
	}

	return pszValue;
}
}

CCVar::CCVar( const char* const pszName, const CCVarArgsBuilder& args )
	: CBaseConCommand( pszName, args.GetFlags(), args.GetHelpInfo() )
	, m_flMinValue( args.GetMinValue() )
	, m_flMaxValue( args.GetMaxValue() )
	, m_bHasMinValue( args.HasMinValue() )
	, m_bHasMaxValue( args.HasMaxValue() )
{
	if( args.UsingStringValue() )
	{
		SetString( args.GetStringValue() );
	}
	else
	{
		SetFloat( args.GetFloatValue() );
	}

	//Set the callback after init to avoid any problems.
	switch( args.GetCallbackType() )
	{
	case CallbackType::FUNCTION:
		{
			SetCallbackFn( args.GetCallbackFn() );
			break;
		}

	case CallbackType::INTERFACE:
		{
			SetHandler( args.GetHandler() );
			break;
		}

	default:
		{
			assert( !"CCVar::CCVar: Invalid callback type!" );
			break;
		}
	}
}

CCVar::~CCVar()
{
	delete[] m_pszValue;
}

void CCVar::SetString( const char* const pszValue )
{
	assert( pszValue );

	char* pszOldValue = m_pszValue;

	const float flOldValue = m_flValue;

	float flValue = m_flValue = static_cast<float>( atof( pszValue ) );

	Clamp( m_flValue );

	//Value was clamped, modify string.
	if( m_flValue != flValue )
	{
		m_pszValue = CreateStringValue( "%.2f", m_flValue );
	}
	else
	{
		//No clamping performed, so just use the string.
		const size_t uiLength = strlen( pszValue );

		m_pszValue = new char[ uiLength + 1 ];

		strcpy( m_pszValue, pszValue );
	}

	ValueChanged( pszOldValue, flOldValue );

	delete[] pszOldValue;
}

void CCVar::Clamp( float& flValue )
{
	if( m_bHasMinValue && flValue < m_flMinValue )
	{
		flValue = m_flMinValue;
	}

	if( m_bHasMaxValue && flValue > m_flMaxValue )
	{
		flValue = m_flMaxValue;
	}
}

void CCVar::SetFloatValue( float flValue, const bool bFormatFloat )
{
	Clamp( flValue );

	const float flOldValue = m_flValue;

	m_flValue = flValue;

	char* pszOldValue = m_pszValue;

	if( bFormatFloat )
	{
		m_pszValue = CreateStringValue( "%.2f", flValue );
	}
	else
	{
		const int iValue = static_cast<int>( flValue );

		m_pszValue = CreateStringValue( "%d", iValue );
	}

	ValueChanged( pszOldValue, flOldValue );

	delete[] pszOldValue;
}

void CCVar::ValueChanged( const char* pszOldValue, const float flOldValue )
{
	if( !pszOldValue )
		pszOldValue = "";

	//Invoke the callback.
	switch( m_CallbackType )
	{
	case CallbackType::FUNCTION:
		{
			if( m_CallbackFn )
			{
				m_CallbackFn( *this, pszOldValue, flOldValue );
			}

			break;
		}

	case CallbackType::INTERFACE:
		{
			if( m_pCallbackObj )
			{
				m_pCallbackObj->HandleCVar( *this, pszOldValue, flOldValue );
			}

			break;
		}

	default:
		{
			assert( !"CCVar::ValueChanged: Invalid callback type!" );

			break;
		}
	}

	cvars().CVarChanged( *this, pszOldValue, flOldValue );
}
}