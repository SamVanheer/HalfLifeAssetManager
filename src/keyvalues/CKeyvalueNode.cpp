#include <cassert>

#include "CKeyvalueNode.h"

namespace keyvalues
{
CKeyvalueNode::CKeyvalueNode( const char* const pszKey, const NodeType type )
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
}