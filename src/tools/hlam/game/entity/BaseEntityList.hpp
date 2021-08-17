#pragma once

#include <cstddef>
#include <vector>

#include "game/entity/EntityConstants.hpp"

class BaseEntity;
class EHandle;

/**
*	@brief Manages a list of entities
*/
class BaseEntityList
{
private:
	/**
	*	@brief Each slot has a serial number that lets us refer to entities safely.
	*	If m_Entities[index].serial == serial, the entity is still what we previously knew it was.
	*/
	struct EntData final
	{
		BaseEntity* Entity = nullptr;
		std::size_t Serial = 0;
	};

public:
	BaseEntityList() = default;
	~BaseEntityList() = default;

	BaseEntityList(const BaseEntityList&) = delete;
	BaseEntityList& operator=(const BaseEntityList&) = delete;

	std::size_t GetNumEntities() const { return _numEntities; }

	BaseEntity* GetEntityByIndex(std::size_t index) const;

	BaseEntity* GetEntityByHandle(const EHandle& handle) const;

	EHandle GetFirstEntity() const;

	EHandle GetNextEntity(const EHandle& previous) const;

	void Add(BaseEntity* entity);

	void Remove(BaseEntity* entity);

	void RemoveAll();

protected:
	/**
	*	@brief Called when an entity has just been added to the list
	*/
	virtual void OnAdd(BaseEntity* entity) {}

	/**
	*	@brief Called right before the entity is removed from the list
	*/
	virtual void OnRemove(BaseEntity* entity) {}

private:
	void FinishAddEntity(std::size_t index, BaseEntity* entity);

	void FinishRemoveEntity(BaseEntity* entity);

private:
	std::vector<EntData> _entities;

	/**
	*	@brief The total number of entity slots currently in use
	*/
	std::size_t _numEntities = 0;
};
