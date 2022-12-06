#pragma once

#include <cassert>
#include <cstddef>
#include <memory>
#include <vector>

#include "entity/EntityConstants.hpp"

class BaseEntity;
class EntityList;
class EHandle;
class WorldTime;

/**
*	@brief Helper class to automatically spawn entities and destroy them if necessary
*/
template<typename TEntity>
struct EntityConstructor final
{
	EntityConstructor(EntityList& list, TEntity* entity)
		: List(list)
		, _entity(entity)
	{
	}

	~EntityConstructor()
	{
		Spawn();
	}

	TEntity* SpawnAndGetEntity()
	{
		Spawn();
		return _entity;
	}

	/**
	*	@brief Spawns the entity now, and if necessary destroys it
	*/
	void Spawn();

	/**
	*	@brief If the entity is valid, invokes @p callable on it
	*/
	template<typename Callable>
	EntityConstructor& operator()(Callable callable)
	{
		if (_entity)
		{
			callable(_entity);
		}

		return *this;
	}

	EntityList& List;

private:
	TEntity* _entity;
	bool _hasSpawned = false;
};

/**
*	@brief Manages a list of entities
*/
class EntityList final
{
private:
	/**
	*	@brief Each slot has a serial number that lets us refer to entities safely.
	*	If m_Entities[index].Serial == serial, the entity is still what we previously knew it was.
	*/
	struct EntData final
	{
		EntData() = default;
		EntData(const EntData&) = delete;
		EntData& operator=(const EntData&) = delete;

		EntData(EntData&&) = default;
		EntData& operator=(EntData&&) = default;

		//If the entity is still non-null on destruct then the entire list is being destroyed, so we should just go ahead and destroy the entity
		//OnDestroy will not be called here, that's up to the user of the list to do before we get here
		std::unique_ptr<BaseEntity> Entity;
		std::size_t Serial = 0;
	};

public:
	EntityList(WorldTime* worldTime)
		: _worldTime(worldTime)
	{
		assert(_worldTime);
	}

	~EntityList() = default;

	EntityList(const EntityList&) = delete;
	EntityList& operator=(const EntityList&) = delete;

	std::size_t GetNumEntities() const { return _numEntities; }

	BaseEntity* GetEntityByIndex(std::size_t index) const;

	BaseEntity* GetEntityByHandle(const EHandle& handle) const;

	EHandle GetFirstEntity() const;

	EHandle GetNextEntity(const EHandle& previous) const;

	/**
	*	@brief Runs a single frame for all entities. Removes entities flagged as needing removal
	*/
	void RunFrame();

	/**
	*	@brief Creates an entity of type @p TEntity with constructor arguments @p args
	*	The resulting object can be used to finish creating the entity
	*	Call SpawnAndGetEntity to spawn the entity and, if the entity has not flagged itself for removal, get the entity
	*	Otherwise let the constructor object go out of scope to automatically spawn the entity
	*/
	template<typename TEntity, typename... Args, typename = std::enable_if_t<std::is_base_of_v<BaseEntity, TEntity>>>
	EntityConstructor<TEntity> Create(Args&&... args)
	{
		auto entity = std::make_unique<TEntity>(std::forward(args)...);

		auto entityPointer = entity.get();

		Add(std::move(entity));

		return {*this, entityPointer};
	}

	void Destroy(BaseEntity* entity);

	void DestroyAll();

private:
	void Add(std::unique_ptr<BaseEntity>&& entity);

private:
	std::vector<EntData> _entities;

	/**
	*	@brief The total number of entity slots currently in use
	*/
	std::size_t _numEntities = 0;

	WorldTime* const _worldTime;
};

template<typename TEntity>
inline void EntityConstructor<TEntity>::Spawn()
{
	if (_entity && !_hasSpawned)
	{
		_hasSpawned = true;
		_entity->Spawn();

		if (_entity->GetFlags() & entity::FL_KILLME)
		{
			List.Destroy(_entity);
			_entity = nullptr;
		}
	}
}

