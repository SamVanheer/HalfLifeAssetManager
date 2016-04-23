#ifndef GAME_ENTITY_CBASEENTITYLIST_H
#define GAME_ENTITY_CBASEENTITYLIST_H

class CBaseEntity;

/**
*	Manages a list of entities.
*/
class CBaseEntityList
{
public:
	static const size_t MAX_ENTITIES = 8192;

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

	CBaseEntity* GetEntityByIndex( const size_t uiIndex ) const;

	CBaseEntity* GetFirstEntity() const;
	CBaseEntity* GetNextEntity( CBaseEntity* pStart ) const;

	CBaseEntity* Create( const char* const pszClassName );

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
	virtual void OnAdded( CBaseEntity* pEntity ) {}

	virtual void OnRemove( CBaseEntity* pEntity ) {}

private:
	void FinishAddEntity( const size_t uiIndex, CBaseEntity* pEntity );
	void FinishRemoveEntity( CBaseEntity* pEntity );

private:
	/**
	*	The actual list.
	*/
	CBaseEntity* m_Entities[ MAX_ENTITIES ];

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