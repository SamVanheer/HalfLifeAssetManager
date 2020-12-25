#include <cassert>

#include "shared/Logging.hpp"
#include "shared/WorldTime.hpp"

#include "game/entity/BaseEntity.hpp"
#include "game/entity/BaseEntityList.hpp"
#include "game/entity/EntityManager.hpp"

EntityManager::EntityManager(std::unique_ptr<BaseEntityList>&& entityList, WorldTime* worldTime)
	: _entityList(std::move(entityList))
	, _worldTime(worldTime)
{
	assert(nullptr != _entityList);
}

EntityManager::~EntityManager() = default;

bool EntityManager::Initialize()
{
	return true;
}

void EntityManager::Shutdown()
{
}

bool EntityManager::OnMapBegin()
{
	assert( !m_bMapRunning );

	m_bMapRunning = true;

	return true;
}

void EntityManager::OnMapEnd()
{
	assert( m_bMapRunning );

	m_bMapRunning = false;

	_entityList->RemoveAll();
}

void EntityManager::RunFrame()
{
	for( EHandle entity = _entityList->GetFirstEntity(); entity.IsValid(*_entityList); entity = _entityList->GetNextEntity( entity ) )
	{
		CBaseEntity* pEntity = entity.Get(*_entityList);

		if( pEntity->AnyFlagsSet( entity::FL_ALWAYSTHINK ) ||
			( pEntity->GetNextThinkTime() != 0 && 
			pEntity->GetNextThinkTime() <= _worldTime->GetCurrentTime() && 
			(_worldTime->GetCurrentTime() - _worldTime->GetFrameTime() ) >= pEntity->GetLastThinkTime() ) )
		{
			//Set first so entities can do lastthink + delay.
			pEntity->SetLastThinkTime(_worldTime->GetCurrentTime() );
			pEntity->SetNextThinkTime( 0 );

			pEntity->Think();
		}
	}

	//Remove all entities flagged with FL_KILLME.
	for( EHandle entity = _entityList->GetFirstEntity(); entity.IsValid(*_entityList); entity = _entityList->GetNextEntity( entity ) )
	{
		auto baseEntity = entity.Get(*_entityList);

		if(baseEntity->GetFlags() & entity::FL_KILLME )
		{
			_entityList->Remove(baseEntity);
		}
	}
}

CBaseEntity* EntityManager::Create(const char* const pszClassName, EntityContext* context,
	const glm::vec3& vecOrigin, const glm::vec3& vecAngles, const bool bSpawn)
{
	CBaseEntity* pEntity = GetEntityDict().CreateEntity(pszClassName, context);

	//This is where you can handle custom entities.
	if (!pEntity)
	{
		Error("Couldn't create \"%s\"!\n", pszClassName);
		return nullptr;
	}

	if (_entityList->Add(pEntity) == entity::INVALID_ENTITY_INDEX)
	{
		GetEntityDict().DestroyEntity(pEntity);

		return nullptr;
	}

	pEntity->SetOrigin(vecOrigin);
	pEntity->SetAngles(vecAngles);

	if (bSpawn)
	{
		if (!pEntity->Spawn())
		{
			_entityList->Remove(pEntity);
			return nullptr;
		}
	}

	return pEntity;
}
