#include <cassert>
#include <memory>

#include "core/shared/Logging.hpp"

#include "game/entity/BaseEntity.hpp"
#include "game/entity/BaseEntityList.hpp"
#include "game/entity/EHandle.hpp"
#include "game/entity/EntityDict.hpp"

BaseEntityList::BaseEntityList()
{
	memset(_entities, 0, sizeof(_entities));
}

BaseEntityList::~BaseEntityList() = default;

BaseEntity* BaseEntityList::GetEntityByIndex(const entity::EntIndex index) const
{
	assert(index < entity::MAX_ENTITIES);

	return _entities[index].Entity;
}

BaseEntity* BaseEntityList::GetEntityByHandle(const EHandle& handle) const
{
	//If it's explicitly invalid, we can ignore it.
	if (handle.GetEntHandle() == entity::INVALID_ENTITY_HANDLE)
	{
		return nullptr;
	}

	assert(handle.GetEntIndex() < entity::MAX_ENTITIES);

	if (_entities[handle.GetEntIndex()].Serial == handle.GetSerialNumber())
	{
		return _entities[handle.GetEntIndex()].Entity;
	}

	return nullptr;
}

EHandle BaseEntityList::GetFirstEntity() const
{
	return GetNextEntity(nullptr);
}

EHandle BaseEntityList::GetNextEntity(const EHandle& previous) const
{
	for (size_t index = previous.IsValid(*this) ? previous.GetEntIndex() + 1 : 0; index < _highestEntIndex; ++index)
	{
		if (_entities[index].Entity)
		{
			return _entities[index].Entity;
		}
	}

	return nullptr;
}

size_t BaseEntityList::Add(BaseEntity* entity)
{
	assert(entity);

	if (_numEntities >= entity::MAX_ENTITIES)
	{
		Warning("Max entities reached (%u)!\n", entity::MAX_ENTITIES);
		return entity::INVALID_ENTITY_INDEX;
	}

	entity::EntIndex index;

	for (index = 0; index < entity::MAX_ENTITIES; ++index)
	{
		if (!_entities[index].Entity)
		{
			break;
		}
	}

	//Shouldn't happen.
	if (index >= entity::MAX_ENTITIES)
	{
		Warning("Max entities reached (%u)!\n", entity::MAX_ENTITIES);
		return entity::INVALID_ENTITY_INDEX;
	}

	++_numEntities;

	if (index >= _highestEntIndex)
	{
		_highestEntIndex = index + 1;
	}

	FinishAddEntity(index, entity);

	return index;
}

void BaseEntityList::Remove(BaseEntity* entity)
{
	if (!entity)
	{
		return;
	}

	const EHandle handle = entity->GetEntHandle();

	const entity::EntIndex uiIndex = handle.GetEntIndex();

	//this shouldn't ever be hit, unless the entity was corrupted/not managed by this list.
	assert(uiIndex < _highestEntIndex);

	//Sanity check.
	assert(_entities[uiIndex].Entity == entity);

	FinishRemoveEntity(entity);

	--_numEntities;

	//Adjust highest entity index.
	if (uiIndex == _highestEntIndex - 1)
	{
		for (; _highestEntIndex > 0; --_highestEntIndex)
		{
			if (_entities[_highestEntIndex - 1].Entity)
			{
				_highestEntIndex = _highestEntIndex - 1;
				break;
			}
		}
	}

	return;
}

void BaseEntityList::RemoveAll()
{
	for (size_t uiIndex = 0; uiIndex < _highestEntIndex; ++uiIndex)
	{
		if (BaseEntity* entity = _entities[uiIndex].Entity; entity)
		{
			FinishRemoveEntity(entity);
		}
	}

	_highestEntIndex = 0;
	_numEntities = 0;

	memset(_entities, 0, sizeof(_entities));
}

void BaseEntityList::FinishAddEntity(const entity::EntIndex index, BaseEntity* entity)
{
	_entities[index].Entity = entity;

	//Increment the serial number to indicate that a new entity is using the slot.
	++_entities[index].Serial;

	EHandle handle;

	handle.SetEntHandle(entity::MakeEntHandle(index, _entities[index].Serial));

	entity->SetEntHandle(handle);

	OnAdded(entity);
}

void BaseEntityList::FinishRemoveEntity(BaseEntity* entity)
{
	OnRemove(entity);

	const EHandle handle = entity->GetEntHandle();

	GetEntityDict().DestroyEntity(entity);

	_entities[handle.GetEntIndex()].Entity = nullptr;
}
