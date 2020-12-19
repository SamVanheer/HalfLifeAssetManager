#ifndef GAME_ENTITY_EHANDLE_H
#define GAME_ENTITY_EHANDLE_H

#include "EntityConstants.hpp"

class CBaseEntity;
class CBaseEntityList;

/**
*	Class that represents an entity. Can be used to safely store references to entities.
*/
class EHandle
{
protected:
	friend class CBaseEntityList;

public:
	/**
	*	Creates a handle that represents no entity.
	*/
	EHandle() = default;

	/**
	*	Creates a handle that represents the given entity.
	*	@param pEntity Entity that this handle should represent.
	*/
	EHandle( CBaseEntity* pEntity );

	/**
	*	Copy constructor.
	*/
	EHandle( const EHandle& other ) = default;

	/**
	*	Assignment operator.
	*/
	EHandle& operator=( const EHandle& other ) = default;

	/**
	*	Invalidates this handle.
	*/
	void Invalidate();

	/**
	*	Gets the entity that this handle represents, or null.
	*/
	CBaseEntity* Get(const CBaseEntityList& entityList) const;

	/**
	*	Returns whether this handle represents a valid entity.
	*/
	bool IsValid(const CBaseEntityList& entityList) const { return Get(entityList) != nullptr; }

	/**
	*	Sets the entity that this handle represents.
	*/
	void Set( CBaseEntity* pEntity );

	/**
	*	@copydoc Set( CBaseEntity* pEntity )
	*/
	EHandle& operator=( CBaseEntity* pEntity );

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
	entity::EntHandle_t GetEntHandle() const { return m_Handle; }

protected:
	/**
	*	Sets the handle value. Only the entity list should access this.
	*/
	void SetEntHandle( const entity::EntHandle_t handle ) { m_Handle = handle; }

private:
	entity::EntHandle_t m_Handle = entity::INVALID_ENTITY_HANDLE;
};

#endif //GAME_ENTITY_EHANDLE_H
