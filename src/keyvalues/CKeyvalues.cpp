#include <cassert>

#include "CKeyvalues.h"

CKeyvalues::CKeyvalues()
{
}

CKeyvalues::CKeyvalues( CKvBlockNode* pRootBlock )
{
	assert( pRootBlock );

	SetRoot( pRootBlock );
}

CKeyvalues::~CKeyvalues()
{
	Reset();
}

void CKeyvalues::SetRoot( CKvBlockNode* pRootBlock )
{
	if( m_pRootBlock )
	{
		delete m_pRootBlock;
	}

	if( pRootBlock )
	{
		m_pRootBlock = pRootBlock;
	}
	else
	{
		m_pRootBlock = nullptr;
	}
}

void CKeyvalues::Reset()
{
	SetRoot( nullptr );
}