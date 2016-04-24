#include <cassert>

#include "common/Logging.h"

#include "CKeyvalue.h"
#include "CKvBlockNode.h"

CKvBlockNode::CKvBlockNode( const char* pszKey )
	: BaseClass( pszKey, KVNode_Block )
{
}

CKvBlockNode::CKvBlockNode( const char* pszKey, const Children_t& children )
	: BaseClass( pszKey, KVNode_Block )
{
	SetChildren( children );
}

CKvBlockNode::CKvBlockNode( const char* pszKey, CKeyvalueNode* pFirstChild )
	: BaseClass( pszKey, KVNode_Block )
{
	assert( pFirstChild );

	m_Children.push_back( pFirstChild );
}

CKvBlockNode::~CKvBlockNode()
{
	RemoveAllChildren();
}

void CKvBlockNode::SetChildren( const Children_t& children )
{
	m_Children = children;
}

void CKvBlockNode::RemoveAllChildren()
{
	for( auto pChild : m_Children )
	{
		delete pChild;
	}

	m_Children.clear();
}

void CKvBlockNode::RemoveAllNotNamed( const char* pszKey )
{
	if( !pszKey || !( *pszKey ) )
		return;

	for( Children_t::iterator it = m_Children.begin(); it != m_Children.end(); )
	{
		if( ( *it )->GetKey() != pszKey )
		{
			delete *it;
			it = m_Children.erase( it );
		}
		else
			++it;
	}
}

CKeyvalueNode* CKvBlockNode::FindFirstChild( const char* pszKey ) const
{
	if( pszKey && *pszKey )
	{
		const Children_t& children = GetChildren();

		for( Children_t::const_iterator it = children.begin(), end = children.end(); it != end; ++it )
		{
			if( strcmp( pszKey, ( *it )->GetKey().CStr() ) == 0 )
				return ( *it );
		}
	}

	return nullptr;
}

CKeyvalueNode* CKvBlockNode::FindFirstChild( const char* pszKey, const KeyvalueNodeType type ) const
{
	if( pszKey && *pszKey )
	{
		const Children_t& children = GetChildren();

		for( Children_t::const_iterator it = children.begin(), end = children.end(); it != end; ++it )
		{
			if( ( *it )->GetType() == type )
			{
				if( strcmp( pszKey, ( *it )->GetKey().CStr() ) == 0 )
					return ( *it );
			}
		}
	}

	return nullptr;
}

CString CKvBlockNode::FindFirstKeyvalue( const char* pszKey ) const
{
	if( pszKey && *pszKey )
	{
		const Children_t& children = GetChildren();

		for( Children_t::const_iterator it = children.begin(), end = children.end(); it != end; ++it )
		{
			if( ( *it )->GetType() == KVNode_Keyvalue )
			{
				CKeyvalue* pKV = static_cast<CKeyvalue*>( *it );

				if( strcmp( pszKey, pKV->GetKey().CStr() ) == 0 )
					return pKV->GetValue();
			}
		}
	}

	return "";
}

void CKvBlockNode::AddKeyvalue( const char* const pszKey, const char* const pszValue )
{
	if( !pszKey || !( *pszKey ) )
		return;

	if( !pszValue ) 
		return;

	m_Children.emplace_back( new CKeyvalue( pszKey, pszValue ) );
}

void CKvBlockNode::Print( const size_t uiTabLevel ) const
{
	Message( "%*s\"%s\"\n%*s{\n", static_cast<int>( uiTabLevel * KEYVALUE_TAB_WIDTH ), "", GetKey().CStr(), static_cast<size_t>( uiTabLevel * KEYVALUE_TAB_WIDTH ), "" );

	PrintChildren( uiTabLevel + 1 );

	Message( "%*s}\n", static_cast<int>( uiTabLevel * KEYVALUE_TAB_WIDTH ), "" );
}

void CKvBlockNode::PrintChildren( const size_t uiTabLevel ) const
{
	for( Children_t::const_iterator it = m_Children.begin(), end = m_Children.end(); it != end; ++it )
		( *it )->Print( uiTabLevel );
}