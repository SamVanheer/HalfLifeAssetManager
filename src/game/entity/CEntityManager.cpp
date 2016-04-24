#include "common/CWorldTime.h"

#include "CBaseEntity.h"
#include "CBaseEntityList.h"

#include "CEntityManager.h"

namespace
{
static CEntityManager g_EntityManager;
}

CEntityManager& EntityManager()
{
	return g_EntityManager;
}

CEntityManager::CEntityManager()
{
}

CEntityManager::~CEntityManager()
{
}

bool CEntityManager::Initialize()
{
	return true;
}

void CEntityManager::Shutdown()
{
}

bool CEntityManager::OnMapBegin()
{
	assert( !m_bMapRunning );

	m_bMapRunning = true;

	return true;
}

void CEntityManager::OnMapEnd()
{
	assert( m_bMapRunning );

	m_bMapRunning = false;

	GetEntityList().RemoveAll();
}

void CEntityManager::RunFrame()
{
	for( EHandle entity = GetEntityList().GetFirstEntity(); entity; entity = GetEntityList().GetNextEntity( entity ) )
	{
		CBaseEntity* pEntity = entity;

		if( pEntity->AnyFlagsSet( entity::FL_ALWAYSTHINK ) ||
			( pEntity->GetNextThinkTime() != 0 && 
			pEntity->GetNextThinkTime() <= WorldTime.GetCurrentTime() && 
			( WorldTime.GetCurrentTime() - WorldTime.GetFrameTime() ) >= pEntity->GetLastThinkTime() ) )
		{
			//Set first so entities can do lastthink + delay.
			pEntity->SetLastThinkTime( WorldTime.GetCurrentTime() );
			pEntity->SetNextThinkTime( 0 );

			pEntity->Think();
		}
	}

	//Remove all entities flagged with FL_KILLME.
	for( EHandle entity = GetEntityList().GetFirstEntity(); entity; entity = GetEntityList().GetNextEntity( entity ) )
	{
		if( entity->GetFlags() & entity::FL_KILLME )
		{
			GetEntityList().Remove( entity );
		}
	}
}