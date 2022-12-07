#pragma once

#include <cassert>
#include <cstddef>
#include <memory>
#include <vector>

#include "entity/EntityConstants.hpp"

class BaseEntity;
class WorldTime;

/**
*	@brief Helper class to automatically spawn entities
*/
template<typename TEntity>
struct EntityConstructor final
{
	explicit EntityConstructor(std::shared_ptr<TEntity>&& entity)
		: _entity(std::move(entity))
	{
	}

	~EntityConstructor()
	{
		Spawn();
	}

	std::shared_ptr<TEntity> SpawnAndGetEntity()
	{
		Spawn();
		return _entity;
	}

	/**
	*	@brief Spawns the entity now
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

private:
	std::shared_ptr<TEntity> _entity;
	bool _hasSpawned = false;
};

/**
*	@brief Manages a list of entities
*/
class EntityList final
{
public:
	explicit EntityList(WorldTime* worldTime)
		: _worldTime(worldTime)
	{
		assert(_worldTime);
	}

	~EntityList() = default;

	EntityList(const EntityList&) = delete;
	EntityList& operator=(const EntityList&) = delete;

	std::size_t GetNumEntities() const { return _entities.size(); }

	std::shared_ptr<BaseEntity> GetEntityByIndex(std::size_t index) const;

	auto begin() const
	{
		return _entities.begin();
	}

	auto end() const
	{
		return _entities.end();
	}

	/**
	*	@brief Runs a single frame for all entities.
	*/
	void RunFrame();

	/**
	*	@brief Creates an entity of type @p TEntity with constructor arguments @p args
	*	The resulting object can be used to finish creating the entity
	*	Call SpawnAndGetEntity to spawn the entity and get the entity
	*	Otherwise let the constructor object go out of scope to automatically spawn the entity
	*/
	template<typename TEntity, typename... Args, typename = std::enable_if_t<std::is_base_of_v<BaseEntity, TEntity>>>
	EntityConstructor<TEntity> Create(Args&&... args)
	{
		auto entity = std::make_shared<TEntity>(std::forward(args)...);

		Add(entity);

		return EntityConstructor{std::move(entity)};
	}

	void Destroy(const std::shared_ptr<BaseEntity>& entity);

	void DestroyAll();

private:
	void Add(const std::shared_ptr<BaseEntity>& entity);

private:
	std::vector<std::shared_ptr<BaseEntity>> _entities;

	WorldTime* const _worldTime;
};

template<typename TEntity>
inline void EntityConstructor<TEntity>::Spawn()
{
	if (_entity && !_hasSpawned)
	{
		_hasSpawned = true;
		_entity->Spawn();
	}
}

