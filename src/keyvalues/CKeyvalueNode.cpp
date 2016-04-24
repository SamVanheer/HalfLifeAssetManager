#include <cassert>

#include "CKeyvalueNode.h"

CKeyvalueNode::CKeyvalueNode( const char* const pszKey, const KeyvalueNodeType type )
	: m_Type( type )
{
	SetKey( pszKey );
}

void CKeyvalueNode::SetKey( const char* const pszKey )
{
	assert( pszKey );

	m_szKey = pszKey;
}

void CKeyvalueNode::SetKey( const CString& szKey )
{
	SetKey( szKey.CStr() );
}