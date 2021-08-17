#include "core/shared/WorldTime.hpp"
#include "entity/BaseEntity.hpp"
#include "entity/EHandle.hpp"
#include "entity/EntityList.hpp"

BaseEntity* EntityList::GetEntityByIndex(std::size_t index) const
{
	if (index >= _entities.size())
	{
		return nullptr;
	}

	return _entities[index].Entity;
}

BaseEntity* EntityList::GetEntityByHandle(const EHandle& handle) const
{
	if (handle.GetIndex() < _entities.size()
		&& _entities[handle.GetIndex()].Serial == handle.GetSerialNumber())
	{
		return _entities[handle.GetIndex()].Entity;
	}

	return nullptr;
}

EHandle EntityList::GetFirstEntity() const
{
	return GetNextEntity(nullptr);
}

EHandle EntityList::GetNextEntity(const EHandle& previous) const
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

void EntityList::RunFrame()
{
	for (EHandle entity = GetFirstEntity(); entity.IsValid(*this); entity = GetNextEntity(entity))
	{
		BaseEntity* pEntity = entity.Get(*this);

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
	for (EHandle entity = GetFirstEntity(); entity.IsValid(*this); entity = GetNextEntity(entity))
	{
		auto baseEntity = entity.Get(*this);

		if (baseEntity->GetFlags() & entity::FL_KILLME)
		{
			Destroy(baseEntity);
		}
	}
}

void EntityList::Add(BaseEntity* entity)
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

	auto& slot = _entities[index];

	slot.Entity = entity;

	//Increment the serial number to indicate that a new entity is using the slot
	++slot.Serial;

	EHandle handle{index, slot.Serial};

	entity->SetEntHandle(handle);
}

void EntityList::Remove(BaseEntity* entity)
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

	_entities[index].Entity = nullptr;

	--_numEntities;
}

void EntityList::DestroyAll()
{
	for (std::size_t index = 0; index < _entities.size(); ++index)
	{
		if (BaseEntity* entity = _entities[index].Entity; entity)
		{
			Destroy(entity);
		}
	}

	_numEntities = 0;

	_entities.clear();
}
