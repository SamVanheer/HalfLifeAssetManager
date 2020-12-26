#pragma once

#include "game/entity/EntityConstants.hpp"

class BaseEntity;
class BaseEntityList;

/**
*	Class that represents an entity. Can be used to safely store references to entities.
*/
class EHandle
{
protected:
	friend class BaseEntityList;

public:
	/**
	*	Creates a handle that represents no entity.
	*/
	EHandle() = default;

	/**
	*	Creates a handle that represents the given entity.
	*	@param entity Entity that this handle should represent.
	*/
	EHandle(BaseEntity* entity);

	/**
	*	Copy constructor.
	*/
	EHandle(const EHandle& other) = default;

	/**
	*	Assignment operator.
	*/
	EHandle& operator=(const EHandle& other) = default;

	/**
	*	Invalidates this handle.
	*/
	void Invalidate();

	/**
	*	Gets the entity that this handle represents, or null.
	*/
	BaseEntity* Get(const BaseEntityList& entityList) const;

	/**
	*	Returns whether this handle represents a valid entity.
	*/
	bool IsValid(const BaseEntityList& entityList) const { return Get(entityList) != nullptr; }

	/**
	*	Sets the entity that this handle represents.
	*/
	void Set(BaseEntity* entity);

	/**
	*	@copydoc Set(BaseEntity* pEntity)
	*/
	EHandle& operator=(BaseEntity* entity);

	/**
	*	Gets the entity's index. Only valid if this points to an entity.
	*/
	entity::EntIndex_t GetEntIndex() const;

	/**
	*	Gets the entity's serial number. Only valid if this points to an entity.
	*/
	entity::EntSerial_t GetSerialNumber() const;

	/**
	*	Gets the EntHandle_t that this handle uses to represent the entity.
	*/
	entity::EntHandle_t GetEntHandle() const { return _handle; }

protected:
	/**
	*	Sets the handle value. Only the entity list should access this.
	*/
	void SetEntHandle(const entity::EntHandle_t handle) { _handle = handle; }

private:
	entity::EntHandle_t _handle = entity::INVALID_ENTITY_HANDLE;
};
