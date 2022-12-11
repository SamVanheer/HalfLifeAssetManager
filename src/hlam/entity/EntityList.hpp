#pragma once

#include <cassert>
#include <concepts>
#include <cstddef>
#include <memory>
#include <vector>

class BaseEntity;
class EntityContext;
class WorldTime;

/**
*	@brief Manages a list of entities
*/
class EntityList final
{
public:
	explicit EntityList(EntityContext* entityContext)
		: _context(entityContext)
	{
		assert(entityContext);
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
	*	@brief Creates an entity of type @p TEntity with constructor arguments @p args.
	*	<tt>Spawn</tt> is called to finish construction.
	*/
	template<std::derived_from<BaseEntity> TEntity, typename... Args>
	std::shared_ptr<TEntity> Create(Args&&... args)
	{
		auto entity = std::make_shared<TEntity>(std::forward<Args>(args)...);

		entity->SetEntityContext(_context);
		entity->SetEntityList(this);

		Add(entity);

		entity->Spawn();

		return entity;
	}

	void Destroy(const std::shared_ptr<BaseEntity>& entity);

	void DestroyAll();

private:
	void Add(const std::shared_ptr<BaseEntity>& entity);

private:
	EntityContext* const _context;

	std::vector<std::shared_ptr<BaseEntity>> _entities;
};
