#include <cassert>

#include "game/entity/BaseEntity.hpp"
#include "game/entity/BaseEntityList.hpp"
#include "game/entity/EHandle.hpp"
#include "game/entity/EntityDict.hpp"

BaseEntity* BaseEntityList::GetEntityByIndex(std::size_t index) const
{
	if (index >= _entities.size())
	{
		return nullptr;
	}

	return _entities[index].Entity;
}

BaseEntity* BaseEntityList::GetEntityByHandle(const EHandle& handle) const
{
	if (handle.GetIndex() < _entities.size()
		&& _entities[handle.GetIndex()].Serial == handle.GetSerialNumber())
	{
		return _entities[handle.GetIndex()].Entity;
	}

	return nullptr;
}

EHandle BaseEntityList::GetFirstEntity() const
{
	return GetNextEntity(nullptr);
}

EHandle BaseEntityList::GetNextEntity(const EHandle& previous) const
{
	for (std::size_t index = previous.IsValid(*this) ? previous.GetIndex() + 1 : 0; index < _entities.size(); ++index)
	{
		if (_entities[index].Entity)
		{
			return _entities[index].Entity;
		}
	}

	return nullptr;
}

void BaseEntityList::Add(BaseEntity* entity)
{
	assert(entity);

	std::size_t index;

	for (index = 0; index < _entities.size(); ++index)
	{
		if (!_entities[index].Entity)
		{
			break;
		}
	}

	if (index == _entities.size())
	{
		_entities.push_back({});
	}

	++_numEntities;

	FinishAddEntity(index, entity);
}

void BaseEntityList::Remove(BaseEntity* entity)
{
	if (!entity)
	{
		return;
	}

	const EHandle handle = entity->GetEntHandle();

	const std::size_t index = handle.GetIndex();

	if (index >= _entities.size())
	{
		//This shouldn't ever be hit, unless the entity was corrupted/not managed by this list
		assert(!"Invalid entity index");
		return;
	}

	//Sanity check
	assert(_entities[index].Entity == entity);

	FinishRemoveEntity(entity);

	--_numEntities;
}

void BaseEntityList::RemoveAll()
{
	for (std::size_t index = 0; index < _entities.size(); ++index)
	{
		if (BaseEntity* entity = _entities[index].Entity; entity)
		{
			FinishRemoveEntity(entity);
		}
	}

	_numEntities = 0;

	_entities.clear();
}

void BaseEntityList::FinishAddEntity(std::size_t index, BaseEntity* entity)
{
	auto& slot = _entities[index];

	slot.Entity = entity;

	//Increment the serial number to indicate that a new entity is using the slot.
	++slot.Serial;

	EHandle handle{index,slot.Serial};

	entity->SetEntHandle(handle);

	OnAdd(entity);
}

void BaseEntityList::FinishRemoveEntity(BaseEntity* entity)
{
	OnRemove(entity);

	const EHandle handle = entity->GetEntHandle();

	//TODO: shouldn't be done here, since the list doesn't create entities
	GetEntityDict().DestroyEntity(entity);

	_entities[handle.GetIndex()].Entity = nullptr;
}
