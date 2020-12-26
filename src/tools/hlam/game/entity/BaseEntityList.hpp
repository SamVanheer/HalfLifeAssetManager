#pragma once

#include "game/entity/EntityConstants.hpp"

class BaseEntity;
class EHandle;

/**
*	Manages a list of entities.
*/
class BaseEntityList
{
private:
	/**
	*	Each slot has a serial number that lets us refer to entities safely.
	*	If m_Entities[ index ].serial == serial, the entity is still what we previously knew it was.
	*/
	struct EntData_t final
	{
		BaseEntity* pEntity;
		entity::EntSerial_t serial;
	};

public:
	BaseEntityList();
	~BaseEntityList();

	/**
	*	Gets the total number of entities.
	*/
	size_t GetNumEntities() const { return _numEntities; }

	/**
	*	Gets the highest entity index that's currently in use, + 1.
	*/
	size_t GetHighestEntityIndex() const { return _highestEntIndex; }

	/**
	*	Gets an entity by index.
	*/
	BaseEntity* GetEntityByIndex(const entity::EntIndex_t uiIndex) const;

	/**
	*	Gets an entity by handle.
	*/
	BaseEntity* GetEntityByHandle(const EHandle& handle) const;

	/**
	*	Gets the first entity in the list.
	*/
	EHandle GetFirstEntity() const;

	/**
	*	Gets the next entity in the list after previous.
	*/
	EHandle GetNextEntity(const EHandle& previous) const;

	/**
	*	Inserts an entity into the list.
	*/
	size_t Add(BaseEntity* pEntity);

	/**
	*	Removes an entity. The entity is destroyed.
	*/
	void Remove(BaseEntity* pEntity);

	/**
	*	Removes all entities. The entities are all destroyed.
	*/
	void RemoveAll();

protected:
	/**
	*	Called when an entity has just been added to the list.
	*/
	virtual void OnAdded(BaseEntity* pEntity) {}

	/**
	*	Called right before the entity is removed from the list.
	*/
	virtual void OnRemove(BaseEntity* pEntity) {}

private:
	/**
	*	Finishes adding an entity.
	*/
	void FinishAddEntity(const entity::EntIndex_t uiIndex, BaseEntity* pEntity);

	/**
	*	Finishes removing an entity.
	*/
	void FinishRemoveEntity(BaseEntity* pEntity);

private:
	/**
	*	The actual list.
	*	TODO: consider: allocate dynamically, resize as needed. Allows for a num_edicts like command line parameter.
	*/
	EntData_t _entities[entity::MAX_ENTITIES];

	/**
	*	The total number of entities.
	*/
	size_t _numEntities = 0;

	/**
	*	The highest entity index that is currently in use, + 1.
	*/
	size_t _highestEntIndex = 0;

private:
	BaseEntityList(const BaseEntityList&) = delete;
	BaseEntityList& operator=(const BaseEntityList&) = delete;
};
