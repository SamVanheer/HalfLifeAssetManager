#include "shared/Logging.h"

#include "CBaseEntity.h"

#include "CEntityDict.h"

CEntityDict& GetEntityDict()
{
	return CEntityDict::GetInstance();
}

CEntityDict& CEntityDict::GetInstance()
{
	//Changed to use this approach so the instance is constructed the first time something needs the dictionary.
	static CEntityDict instance;

	return instance;
}

bool CEntityDict::HasEntity( const char* const pszClassName ) const
{
	return FindEntity( pszClassName ) != nullptr;
}

const CBaseEntityRegistry* CEntityDict::FindEntity( const char* const pszClassName ) const
{
	assert( pszClassName );

	auto it = m_Dict.find( pszClassName );

	return it != m_Dict.end() ? it->second : nullptr;
}

bool CEntityDict::AddEntity( const CBaseEntityRegistry* pRegistry )
{
	assert( pRegistry );

	if( FindEntity( pRegistry->GetClassname() ) )
	{
		//This shouldn't happen, since there'd be duplicate symbols in the library.
		Error( "CEntityDict::AddEntity: Duplicate entity \"%s\" added!\n", pRegistry->GetClassname() );
		return false;
	}

	m_Dict.insert( std::make_pair( pRegistry->GetClassname(), pRegistry ) );

	return true;
}

CBaseEntity* CEntityDict::CreateEntity( const char* const pszClassName, EntityContext* context) const
{
	const auto pReg = FindEntity( pszClassName );

	if( !pReg )
		return nullptr;

	if( CBaseEntity* pEntity = pReg->Create() )
	{
		pEntity->Construct( pszClassName, context);
		pEntity->OnCreate();

		return pEntity;
	}

	return nullptr;
}

void CEntityDict::DestroyEntity( CBaseEntity* pEntity ) const
{
	assert( pEntity );

	const auto pReg = FindEntity( pEntity->GetClassName() );

	if( !pReg )
	{
		Error( "CEntityDict::DestroyEntity: Tried to destroy unknown entity \"%s\"!\n", pEntity->GetClassName() );
		return;
	}

	pEntity->OnDestroy();

	pReg->Destroy( pEntity );
}