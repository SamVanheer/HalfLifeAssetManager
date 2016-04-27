#include <cassert>

#include "shared/Logging.h"

#include "CKeyvalue.h"
#include "CKeyvalueBlock.h"

namespace keyvalues
{
CKeyvalueBlock::CKeyvalueBlock( const char* const pszKey )
	: BaseClass( pszKey, NodeType::BLOCK )
{
}

CKeyvalueBlock::CKeyvalueBlock( const char* const pszKey, const Children_t& children )
	: CKeyvalueBlock( pszKey )
{
	SetChildren( children );
}

CKeyvalueBlock::CKeyvalueBlock( const char* const pszKey, CKeyvalueNode* pFirstChild )
	: CKeyvalueBlock( pszKey )
{
	assert( pFirstChild );

	m_Children.push_back( pFirstChild );
}

CKeyvalueBlock::~CKeyvalueBlock()
{
	RemoveAllChildren();
}

void CKeyvalueBlock::SetChildren( const Children_t& children )
{
	RemoveAllChildren();

	m_Children.reserve( children.size() );

	for( auto pChild : children )
	{
		assert( pChild );

		m_Children.push_back( pChild );
	}
}

void CKeyvalueBlock::RemoveAllChildren()
{
	for( auto pChild : m_Children )
	{
		delete pChild;
	}

	m_Children.clear();
}

void CKeyvalueBlock::RemoveAllNotNamed( const char* const pszKey )
{
	assert( pszKey );

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

CKeyvalueNode* CKeyvalueBlock::FindFirstChild( const char* const pszKey ) const
{
	assert( pszKey );

	for( const auto pChild : m_Children )
	{
		if( strcmp( pszKey, pChild->GetKey().CStr() ) == 0 )
			return pChild;
	}

	return nullptr;
}

CKeyvalueNode* CKeyvalueBlock::FindFirstChild( const char* const pszKey, const NodeType type ) const
{
	assert( pszKey );

	for( const auto pChild : m_Children )
	{
		if( pChild->GetType() == type )
		{
			if( strcmp( pszKey, pChild->GetKey().CStr() ) == 0 )
				return pChild;
		}
	}

	return nullptr;
}

CString CKeyvalueBlock::FindFirstKeyvalue( const char* const pszKey ) const
{
	if( pszKey && *pszKey )
	{
		const Children_t& children = GetChildren();

		for( Children_t::const_iterator it = children.begin(), end = children.end(); it != end; ++it )
		{
			if( ( *it )->GetType() == NodeType::KEYVALUE )
			{
				CKeyvalue* pKV = static_cast<CKeyvalue*>( *it );

				if( strcmp( pszKey, pKV->GetKey().CStr() ) == 0 )
					return pKV->GetValue();
			}
		}
	}

	return "";
}

void CKeyvalueBlock::AddKeyvalue( const char* const pszKey, const char* const pszValue )
{
	assert( pszKey );
	assert( pszValue );

	m_Children.emplace_back( new CKeyvalue( pszKey, pszValue ) );
}

void CKeyvalueBlock::Print( const size_t uiTabLevel ) const
{
	Message( "%*s\"%s\"\n%*s{\n", static_cast<int>( uiTabLevel * KEYVALUE_TAB_WIDTH ), "", GetKey().CStr(), static_cast<size_t>( uiTabLevel * KEYVALUE_TAB_WIDTH ), "" );

	PrintChildren( uiTabLevel + 1 );

	Message( "%*s}\n", static_cast<int>( uiTabLevel * KEYVALUE_TAB_WIDTH ), "" );
}

void CKeyvalueBlock::PrintChildren( const size_t uiTabLevel ) const
{
	for( Children_t::const_iterator it = m_Children.begin(), end = m_Children.end(); it != end; ++it )
		( *it )->Print( uiTabLevel );
}
}