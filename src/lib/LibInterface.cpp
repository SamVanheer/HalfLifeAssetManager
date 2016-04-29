#include <cassert>
#include <cstring>

#include "LibInterface.h"

CInterfaceRegistry* CInterfaceRegistry::m_pHead = nullptr;

CInterfaceRegistry::CInterfaceRegistry( const char* const pszName, const InstantiateInterfaceFn instantiateFn )
	: m_pNext( m_pHead )
	, m_pszName( pszName )
	, m_InstantiateFn( instantiateFn )
{
	assert( pszName && *pszName );
	assert( instantiateFn );

	m_pHead = this;
}

IBaseInterface* CreateInterface( const char* const pszName, IFaceResult* pResult )
{
	IBaseInterface* pInterface = nullptr;
	IFaceResult result = IFaceResult::FAILURE;

	for( auto pIFace = CInterfaceRegistry::GetHead(); pIFace; pIFace = pIFace->GetNext() )
	{
		if( strcmp( pszName, pIFace->GetName() ) == 0 )
		{
			pInterface = pIFace->GetInstantiateFn()();

			result = IFaceResult::SUCCESS;

			break;
		}
	}

	if( pResult )
		*pResult = result;

	return pInterface;
}