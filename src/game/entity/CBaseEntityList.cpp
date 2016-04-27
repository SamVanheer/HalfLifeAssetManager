#include <cassert>
#include <memory>

#include "shared/Logging.h"

#include "CBaseEntity.h"
#include "EHandle.h"

#include "CEntityDict.h"

#include "CBaseEntityList.h"

namespace
{
//TODO: the actual list should be defined by the application.
static CBaseEntityList g_EntityList;
}

CBaseEntityList& GetEntityList()
{
	return g_EntityList;
}

CBaseEntityList::CBaseEntityList()
{
	memset( m_Entities, 0, sizeof( m_Entities ) );
}

CBaseEntityList::~CBaseEntityList()
{
}

CBaseEntity* CBaseEntityList::GetEntityByIndex( const entity::EntIndex_t uiIndex ) const
{
	assert( uiIndex < entity::MAX_ENTITIES );

	return m_Entities[ uiIndex ].pEntity;
}

CBaseEntity* CBaseEntityList::GetEntityByHandle( const EHandle& handle ) const
{
	//If it's explicitly invalid, we can ignore it.
	if( handle.GetEntHandle() == entity::INVALID_ENTITY_HANDLE )
		return nullptr;

	assert( handle.GetEntIndex() < entity::MAX_ENTITIES );

	if( m_Entities[ handle.GetEntIndex() ].serial == handle.GetSerialNumber() )
		return m_Entities[ handle.GetEntIndex() ].pEntity;

	return nullptr;
}

EHandle CBaseEntityList::GetFirstEntity() const
{
	return GetNextEntity( nullptr );
}

EHandle CBaseEntityList::GetNextEntity( const EHandle& previous ) const
{
	for( size_t uiIndex = previous.IsValid() ? previous.GetEntIndex() + 1 : 0; uiIndex < m_uiHighestEntIndex; ++uiIndex )
	{
		if( m_Entities[ uiIndex ].pEntity )
			return m_Entities[ uiIndex ].pEntity;
	}

	return nullptr;
}

size_t CBaseEntityList::Add( CBaseEntity* pEntity )
{
	assert( pEntity );

	if( m_uiNumEntities >= entity::MAX_ENTITIES )
	{
		Warning( "Max entities reached (%u)!\n", entity::MAX_ENTITIES );
		return entity::INVALID_ENTITY_INDEX;
	}

	entity::EntIndex_t uiIndex;

	for( uiIndex = 0; uiIndex < entity::MAX_ENTITIES; ++uiIndex )
	{
		if( !m_Entities[ uiIndex ].pEntity )
			break;
	}

	//Shouldn't happen.
	if( uiIndex >= entity::MAX_ENTITIES )
	{
		Warning( "Max entities reached (%u)!\n", entity::MAX_ENTITIES );
		return entity::INVALID_ENTITY_INDEX;
	}

	++m_uiNumEntities;

	if( uiIndex >= m_uiHighestEntIndex )
		m_uiHighestEntIndex = uiIndex + 1;

	FinishAddEntity( uiIndex, pEntity );

	return uiIndex;
}

void CBaseEntityList::Remove( CBaseEntity* pEntity )
{
	if( !pEntity )
		return;

	const EHandle handle = pEntity->GetEntHandle();

	const entity::EntIndex_t uiIndex = handle.GetEntIndex();

	//this shouldn't ever be hit, unless the entity was corrupted/not managed by this list.
	assert( uiIndex < m_uiHighestEntIndex );

	//Sanity check.
	assert( m_Entities[ uiIndex ].pEntity == pEntity );

	FinishRemoveEntity( pEntity );

	--m_uiNumEntities;

	//Adjust highest entity index.
	if( uiIndex == m_uiHighestEntIndex - 1 )
	{
		for( ; m_uiHighestEntIndex > 0; --m_uiHighestEntIndex )
		{
			if( m_Entities[ m_uiHighestEntIndex - 1 ].pEntity )
			{
				m_uiHighestEntIndex = m_uiHighestEntIndex - 1;
				break;
			}
		}
	}

	return;
}

void CBaseEntityList::RemoveAll()
{
	for( size_t uiIndex = 0; uiIndex < m_uiHighestEntIndex; ++uiIndex )
	{
		if( CBaseEntity* pEntity = m_Entities[ uiIndex ].pEntity )
		{
			FinishRemoveEntity( pEntity );
		}
	}

	m_uiHighestEntIndex = 0;
	m_uiNumEntities = 0;

	memset( m_Entities, 0, sizeof( m_Entities ) );
}

void CBaseEntityList::FinishAddEntity( const entity::EntIndex_t uiIndex, CBaseEntity* pEntity )
{
	m_Entities[ uiIndex ].pEntity = pEntity;

	//Increment the serial number to indicate that a new entity is using the slot.
	++m_Entities[ uiIndex ].serial;

	EHandle handle;

	handle.SetEntHandle( entity::MakeEntHandle( uiIndex, m_Entities[ uiIndex ].serial ) );

	pEntity->SetEntHandle( handle );

	OnAdded( pEntity );
}

void CBaseEntityList::FinishRemoveEntity( CBaseEntity* pEntity )
{
	OnRemove( pEntity );

	const EHandle handle = pEntity->GetEntHandle();

	GetEntityDict().DestroyEntity( pEntity );

	m_Entities[ handle.GetEntIndex() ].pEntity = nullptr;
}