#ifndef GAME_ENTITY_CBASEENTITYLIST_H
#define GAME_ENTITY_CBASEENTITYLIST_H

#include "EntityConstants.h"

class CBaseEntity;
class EHandle;

/**
*	Manages a list of entities.
*/
class CBaseEntityList
{
private:
	/**
	*	Each slot has a serial number that lets us refer to entities safely.
	*	If m_Entities[ index ].serial == serial, the entity is still what we previously knew it was.
	*/
	struct EntData_t final
	{
		CBaseEntity*		pEntity;
		entity::EntSerial_t serial;
	};

public:
	CBaseEntityList();
	~CBaseEntityList();

	/**
	*	Gets the total number of entities.
	*/
	size_t GetNumEntities() const { return m_uiNumEntities; }

	/**
	*	Gets the highest entity index that's currently in use, + 1.
	*/
	size_t GetHighestEntityIndex() const { return m_uiHighestEntIndex; }

	/**
	*	Gets an entity by index.
	*/
	CBaseEntity* GetEntityByIndex( const entity::EntIndex_t uiIndex ) const;

	/**
	*	Gets an entity by handle.
	*/
	CBaseEntity* GetEntityByHandle( const EHandle& handle ) const;

	/**
	*	Gets the first entity in the list.
	*/
	EHandle GetFirstEntity() const;

	/**
	*	Gets the next entity in the list after previous.
	*/
	EHandle GetNextEntity( const EHandle& previous ) const;

	/**
	*	Inserts an entity into the list.
	*/
	size_t Add( CBaseEntity* pEntity );

	/**
	*	Removes an entity. The entity is destroyed.
	*/
	void Remove( CBaseEntity* pEntity );

	/**
	*	Removes all entities. The entities are all destroyed.
	*/
	void RemoveAll();

protected:
	/**
	*	Called when an entity has just been added to the list.
	*/
	virtual void OnAdded( CBaseEntity* pEntity ) {}

	/**
	*	Called right before the entity is removed from the list.
	*/
	virtual void OnRemove( CBaseEntity* pEntity ) {}

private:
	void FinishAddEntity( const entity::EntIndex_t uiIndex, CBaseEntity* pEntity );
	void FinishRemoveEntity( CBaseEntity* pEntity );

private:
	/**
	*	The actual list.
	*	TODO: consider: allocate dynamically, resize as needed. Allows for a num_edicts like command line parameter.
	*/
	EntData_t m_Entities[ entity::MAX_ENTITIES ];

	/**
	*	The total number of entities.
	*/
	size_t m_uiNumEntities = 0;

	/**
	*	The highest entity index that is currently in use, + 1.
	*/
	size_t m_uiHighestEntIndex = 0;

private:
	CBaseEntityList( const CBaseEntityList& ) = delete;
	CBaseEntityList& operator=( const CBaseEntityList& ) = delete;
};

CBaseEntityList& GetEntityList();

#endif //GAME_ENTITY_CBASEENTITYLIST_H