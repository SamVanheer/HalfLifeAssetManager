#include <cassert>

#include "CKeyvalueNode.h"

CKeyvalueNode::CKeyvalueNode( const char* pszKey, const KeyvalueNodeType type )
	: m_Type( type )
{
	assert( pszKey && *pszKey );

	SetKey( pszKey );
}

CKeyvalueNode::CKeyvalueNode( const KeyvalueNodeType type )
	: m_Type( type )
{
}

void CKeyvalueNode::SetKey( const char* pszKey )
{
	if( !pszKey || !( *pszKey ) )
	{
		m_szKey = KEYVALUE_DEFAULT_KEY;
	}
	else
	{
		m_szKey = pszKey;
	}
}

void CKeyvalueNode::SetKey( const CString& szKey )
{
	SetKey( szKey.CStr() );
}