#include <cassert>

#include "core/shared/Logging.hpp"
#include "core/shared/WorldTime.hpp"

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
	assert(!_mapRunning);

	_mapRunning = true;

	return true;
}

void EntityManager::OnMapEnd()
{
	assert(_mapRunning);

	_mapRunning = false;

	_entityList->RemoveAll();
}

void EntityManager::RunFrame()
{
	for (EHandle entity = _entityList->GetFirstEntity(); entity.IsValid(*_entityList); entity = _entityList->GetNextEntity(entity))
	{
		BaseEntity* pEntity = entity.Get(*_entityList);

		if (pEntity->AnyFlagsSet(entity::FL_ALWAYSTHINK) ||
			(pEntity->GetNextThinkTime() != 0 &&
				pEntity->GetNextThinkTime() <= _worldTime->GetTime() &&
				(_worldTime->GetTime() - _worldTime->GetFrameTime()) >= pEntity->GetLastThinkTime()))
		{
			//Set first so entities can do lastthink + delay.
			pEntity->SetLastThinkTime(_worldTime->GetTime());
			pEntity->SetNextThinkTime(0);

			pEntity->Think();
		}
	}

	//Remove all entities flagged with FL_KILLME.
	for (EHandle entity = _entityList->GetFirstEntity(); entity.IsValid(*_entityList); entity = _entityList->GetNextEntity(entity))
	{
		auto baseEntity = entity.Get(*_entityList);

		if (baseEntity->GetFlags() & entity::FL_KILLME)
		{
			_entityList->Remove(baseEntity);
		}
	}
}

BaseEntity* EntityManager::Create(const char* const pszClassName, EntityContext * context,
	const glm::vec3 & origin, const glm::vec3 & angles, const bool bSpawn)
{
	BaseEntity* entity = GetEntityDict().CreateEntity(pszClassName, context);

	//This is where you can handle custom entities.
	if (!entity)
	{
		Error("Couldn't create \"%s\"!\n", pszClassName);
		return nullptr;
	}

	if (_entityList->Add(entity) == entity::INVALID_ENTITY_INDEX)
	{
		GetEntityDict().DestroyEntity(entity);

		return nullptr;
	}

	entity->SetOrigin(origin);
	entity->SetAngles(angles);

	if (bSpawn)
	{
		if (!entity->Spawn())
		{
			_entityList->Remove(entity);
			return nullptr;
		}
	}

	return entity;
}
