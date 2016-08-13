#include <cstdio>
#include <cstring>
#include <limits>

#include "shared/Platform.h"
#include "shared/Logging.h"

#include "CVar.h"

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
	m_pParent = this;

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

CallbackType CCVar::GetCallbackType() const
{
	return m_pParent->m_CallbackType;
}

CVarCallback CCVar::GetCallbackFn() const
{
	return m_pParent->m_CallbackFn;
}

void CCVar::SetCallbackFn( const CVarCallback callbackFn )
{
	m_pParent->m_CallbackFn = callbackFn;

	m_pParent->m_CallbackType = CallbackType::FUNCTION;
}

ICVarHandler* CCVar::GetHandler() const
{
	return m_pParent->m_pCallbackObj;
}

void CCVar::SetHandler( ICVarHandler* pCallbackObj )
{
	m_pParent->m_pCallbackObj = pCallbackObj;

	m_pParent->m_CallbackType = CallbackType::INTERFACE;
}

const char* CCVar::GetString() const
{
	return m_pParent->m_pszValue;
}

float CCVar::GetFloat() const
{
	return m_pParent->m_flValue;
}

int CCVar::GetInt() const
{
	return static_cast<int>( m_pParent->m_flValue );
}

bool CCVar::GetBool() const
{
	return ( m_pParent->m_flValue ) != 0;
}

void CCVar::SetString( const char* const pszValue )
{
	m_pParent->SetStringValue( pszValue );
}

void CCVar::SetFloat( float flValue )
{
	m_pParent->SetFloatValue( flValue, true );
}

void CCVar::SetInt( int iValue )
{
	m_pParent->SetFloatValue( static_cast<float>( iValue ), false );
}

void CCVar::SetBool( const bool bValue )
{
	m_pParent->SetInt( bValue ? 1 : 0 );
}

float CCVar::GetMinValue() const
{
	return m_pParent->m_flMinValue;
}

bool CCVar::HasMinValue() const
{
	return m_pParent->m_bHasMinValue;
}

float CCVar::GetMaxValue() const
{
	return m_pParent->m_flMaxValue;
}

bool CCVar::HasMaxValue() const
{
	return m_pParent->m_bHasMaxValue;
}

void CCVar::Clamp( float& flValue )
{
	if( m_pParent->m_bHasMinValue && flValue < m_pParent->m_flMinValue )
	{
		flValue = m_pParent->m_flMinValue;
	}

	if( m_pParent->m_bHasMaxValue && flValue > m_pParent->m_flMaxValue )
	{
		flValue = m_pParent->m_flMaxValue;
	}
}

void CCVar::SetStringValue( const char* pszValue )
{
	assert( this == m_pParent );

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

void CCVar::SetFloatValue( float flValue, const bool bFormatFloat )
{
	assert( this == m_pParent );

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
	assert( this == m_pParent );

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

	if( g_pCVar )
	{
		g_pCVar->CVarChanged( *this, pszOldValue, flOldValue );
	}
}
}