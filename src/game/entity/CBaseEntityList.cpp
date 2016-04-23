#include <cassert>
#include <memory>

#include "common/Logging.h"

#include "CBaseEntity.h"

#include "CEntityDict.h"

#include "CBaseEntityList.h"

namespace
{
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

CBaseEntity* CBaseEntityList::GetEntityByIndex( const size_t uiIndex ) const
{
	assert( uiIndex < MAX_ENTITIES );

	return m_Entities[ uiIndex ];
}

CBaseEntity* CBaseEntityList::GetFirstEntity() const
{
	return GetNextEntity( nullptr );
}

CBaseEntity* CBaseEntityList::GetNextEntity( CBaseEntity* pStart ) const
{
	for( size_t uiIndex = pStart ? pStart->GetEntIndex() : 0; uiIndex < m_uiHighestEntIndex; ++uiIndex )
	{
		if( m_Entities[ uiIndex ] )
			return m_Entities[ uiIndex ];
	}

	return nullptr;
}

CBaseEntity* CBaseEntityList::Create( const char* const pszClassName )
{
	if( CBaseEntity* pEntity = GetEntityDict().CreateEntity( pszClassName ) )
	{
		if( Add( pEntity ) != entity::INVALID_ENTITY_INDEX )
		{
			return pEntity;
		}

		GetEntityDict().DestroyEntity( pEntity );

		return nullptr;
	}

	return nullptr;
}

size_t CBaseEntityList::Add( CBaseEntity* pEntity )
{
	assert( pEntity );

	if( m_uiNumEntities >= MAX_ENTITIES )
	{
		Warning( "Max entities reached (%u)!\n", MAX_ENTITIES );
		return entity::INVALID_ENTITY_INDEX;
	}

	size_t uiIndex;

	for( uiIndex = 0; uiIndex < MAX_ENTITIES; ++uiIndex )
	{
		if( !m_Entities[ uiIndex ] )
			break;
	}

	//Shouldn't happen.
	if( uiIndex >= MAX_ENTITIES )
	{
		Warning( "Max entities reached (%u)!\n", MAX_ENTITIES );
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

	const size_t uiIndex = pEntity->GetEntIndex();

	//this shouldn't ever be hit, unless the entity was corrupted/not managed by this list.
	assert( uiIndex < m_uiHighestEntIndex );

	//Sanity check.
	assert( m_Entities[ uiIndex ] == pEntity );

	FinishRemoveEntity( pEntity );

	--m_uiNumEntities;

	//Adjust highest entity index.
	if( uiIndex == m_uiHighestEntIndex - 1 )
	{
		for( ; m_uiHighestEntIndex > 0; --m_uiHighestEntIndex )
		{
			if( m_Entities[ m_uiHighestEntIndex - 1 ] )
			{
				m_uiHighestEntIndex = m_uiHighestEntIndex - 1;
				break;
			}
		}
	}
}

void CBaseEntityList::RemoveAll()
{
	for( size_t uiIndex = 0; uiIndex < m_uiHighestEntIndex; ++uiIndex )
	{
		if( CBaseEntity* pEntity = m_Entities[ uiIndex ] )
		{
			FinishRemoveEntity( pEntity );
		}
	}

	m_uiHighestEntIndex = 0;
	m_uiNumEntities = 0;
}

void CBaseEntityList::FinishAddEntity( const size_t uiIndex, CBaseEntity* pEntity )
{
	m_Entities[ uiIndex ] = pEntity;
	pEntity->SetEntIndex( uiIndex );

	OnAdded( pEntity );
}

void CBaseEntityList::FinishRemoveEntity( CBaseEntity* pEntity )
{
	OnRemove( pEntity );

	const size_t uiIndex = pEntity->GetEntIndex();

	GetEntityDict().DestroyEntity( pEntity );

	m_Entities[ uiIndex ] = nullptr;
}