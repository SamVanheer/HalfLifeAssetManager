#include "core/shared/Logging.hpp"

#include "game/entity/BaseEntity.hpp"
#include "game/entity/EntityDict.hpp"

EntityDict& GetEntityDict()
{
	return EntityDict::GetInstance();
}

EntityDict& EntityDict::GetInstance()
{
	//Changed to use this approach so the instance is constructed the first time something needs the dictionary.
	static EntityDict instance;

	return instance;
}

bool EntityDict::HasEntity( const char* const pszClassName ) const
{
	return FindEntity( pszClassName ) != nullptr;
}

const CBaseEntityRegistry* EntityDict::FindEntity( const char* const pszClassName ) const
{
	assert( pszClassName );

	auto it = m_Dict.find( pszClassName );

	return it != m_Dict.end() ? it->second : nullptr;
}

bool EntityDict::AddEntity( const CBaseEntityRegistry* pRegistry )
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

CBaseEntity* EntityDict::CreateEntity( const char* const pszClassName, EntityContext* context) const
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

void EntityDict::DestroyEntity( CBaseEntity* pEntity ) const
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