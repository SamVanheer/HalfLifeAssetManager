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

BaseEntity* BaseEntityList::GetEntityByIndex(const entity::EntIndex_t uiIndex) const
{
	assert(uiIndex < entity::MAX_ENTITIES);

	return _entities[uiIndex].pEntity;
}

BaseEntity* BaseEntityList::GetEntityByHandle(const EHandle& handle) const
{
	//If it's explicitly invalid, we can ignore it.
	if (handle.GetEntHandle() == entity::INVALID_ENTITY_HANDLE)
	{
		return nullptr;
	}

	assert(handle.GetEntIndex() < entity::MAX_ENTITIES);

	if (_entities[handle.GetEntIndex()].serial == handle.GetSerialNumber())
	{
		return _entities[handle.GetEntIndex()].pEntity;
	}

	return nullptr;
}

EHandle BaseEntityList::GetFirstEntity() const
{
	return GetNextEntity(nullptr);
}

EHandle BaseEntityList::GetNextEntity(const EHandle& previous) const
{
	for (size_t uiIndex = previous.IsValid(*this) ? previous.GetEntIndex() + 1 : 0; uiIndex < _highestEntIndex; ++uiIndex)
	{
		if (_entities[uiIndex].pEntity)
		{
			return _entities[uiIndex].pEntity;
		}
	}

	return nullptr;
}

size_t BaseEntityList::Add(BaseEntity* pEntity)
{
	assert(pEntity);

	if (_numEntities >= entity::MAX_ENTITIES)
	{
		Warning("Max entities reached (%u)!\n", entity::MAX_ENTITIES);
		return entity::INVALID_ENTITY_INDEX;
	}

	entity::EntIndex_t uiIndex;

	for (uiIndex = 0; uiIndex < entity::MAX_ENTITIES; ++uiIndex)
	{
		if (!_entities[uiIndex].pEntity)
		{
			break;
		}
	}

	//Shouldn't happen.
	if (uiIndex >= entity::MAX_ENTITIES)
	{
		Warning("Max entities reached (%u)!\n", entity::MAX_ENTITIES);
		return entity::INVALID_ENTITY_INDEX;
	}

	++_numEntities;

	if (uiIndex >= _highestEntIndex)
	{
		_highestEntIndex = uiIndex + 1;
	}

	FinishAddEntity(uiIndex, pEntity);

	return uiIndex;
}

void BaseEntityList::Remove(BaseEntity* pEntity)
{
	if (!pEntity)
		return;

	const EHandle handle = pEntity->GetEntHandle();

	const entity::EntIndex_t uiIndex = handle.GetEntIndex();

	//this shouldn't ever be hit, unless the entity was corrupted/not managed by this list.
	assert(uiIndex < _highestEntIndex);

	//Sanity check.
	assert(_entities[uiIndex].pEntity == pEntity);

	FinishRemoveEntity(pEntity);

	--_numEntities;

	//Adjust highest entity index.
	if (uiIndex == _highestEntIndex - 1)
	{
		for (; _highestEntIndex > 0; --_highestEntIndex)
		{
			if (_entities[_highestEntIndex - 1].pEntity)
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
		if (BaseEntity* pEntity = _entities[uiIndex].pEntity)
		{
			FinishRemoveEntity(pEntity);
		}
	}

	_highestEntIndex = 0;
	_numEntities = 0;

	memset(_entities, 0, sizeof(_entities));
}

void BaseEntityList::FinishAddEntity(const entity::EntIndex_t uiIndex, BaseEntity* pEntity)
{
	_entities[uiIndex].pEntity = pEntity;

	//Increment the serial number to indicate that a new entity is using the slot.
	++_entities[uiIndex].serial;

	EHandle handle;

	handle.SetEntHandle(entity::MakeEntHandle(uiIndex, _entities[uiIndex].serial));

	pEntity->SetEntHandle(handle);

	OnAdded(pEntity);
}

void BaseEntityList::FinishRemoveEntity(BaseEntity* pEntity)
{
	OnRemove(pEntity);

	const EHandle handle = pEntity->GetEntHandle();

	GetEntityDict().DestroyEntity(pEntity);

	_entities[handle.GetEntIndex()].pEntity = nullptr;
}
