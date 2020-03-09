#include <cassert>

#include "shared/Logging.h"

#include "CKeyvalue.h"

namespace keyvalues
{
CKeyvalue::CKeyvalue( const char* const pszKey, const char* const pszValue )
	: CKeyvalueNode( pszKey, NodeType::KEYVALUE )
{
	SetValue( pszValue );
}

void CKeyvalue::SetValue( const char* const pszValue )
{
	assert( pszValue );

	m_szValue = pszValue;
}

void CKeyvalue::SetValue( const std::string& szValue )
{
	SetValue( szValue.c_str() );
}

void CKeyvalue::Print( const size_t uiTabLevel ) const
{
	Message( "%*s\"%s\" \"%s\"\n", static_cast<int>( uiTabLevel * KEYVALUE_TAB_WIDTH ), "", GetKey().c_str(), m_szValue.c_str() );
}
}