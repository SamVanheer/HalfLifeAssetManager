#include "CKeyvalues.h"

CKeyvalues::CKeyvalues()
{
}

CKeyvalues::CKeyvalues( std::shared_ptr<CKvBlockNode> rootBlock )
{
	SetRoot( rootBlock );
}

void CKeyvalues::SetRoot( std::shared_ptr<CKvBlockNode> rootBlock )
{
	m_RootBlock = rootBlock;
}

void CKeyvalues::Reset()
{
	m_RootBlock.reset();
}