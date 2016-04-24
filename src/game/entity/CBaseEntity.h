#ifndef GAME_CBASEENTITY_H
#define GAME_CBASEENTITY_H

#include <glm/vec3.hpp>

#include "common/Class.h"

#include "EntityConstants.h"
#include "EHandle.h"
#include "CEntityDict.h"

//Windows defines this
#ifdef GetClassName
#undef GetClassName
#endif

class CBaseEntity;

using ThinkFunc_t = void ( CBaseEntity::* )();

/**
*	Base class for all entities.
*/
class CBaseEntity
{
public:
	DECLARE_CLASS_NOBASE( CBaseEntity );

public:
	CBaseEntity();
	~CBaseEntity();

	/**
	*	Called when the object has been fully constructed.
	*/
	virtual void OnCreate();

	/**
	*	Called right before the object is to be destroyed.
	*/
	virtual void OnDestroy();

	/**
	*	Called when the entity has finished initializing.
	*	@return true on success, false if the entity should be destroyed.
	*/
	virtual bool Spawn();

	/**
	*	Draws this entity.
	*/
	virtual void Draw( entity::DrawFlags_t flags ) {}

private:
	const char* m_pszClassName = nullptr;
	EHandle m_EntHandle;

public:
	/**
	*	Called when the entity is constructed. Do not call directly.
	*	@param pszClassName The entity's class name.
	*/
	void Construct( const char* const pszClassName );

	/**
	*	Gets the entity's class name.
	*/
	const char* GetClassName() const { return m_pszClassName; }

	/**
	*	Gets the handle that represents this entity.
	*/
	const EHandle& GetEntHandle() const { return m_EntHandle; }

	/**
	*	Sets the handle that represents this entity. Should only be used by the entity list.
	*/
	void SetEntHandle( const EHandle& handle ) { m_EntHandle = handle; }

	/**
	*	Creates a new entity by classname. This is the one place where entities can be created.
	*	@param pszClassName The entity's class name.
	*	@param vecOrigin The entity's origin.
	*	@param vecAngles The entity's angles.
	*	@param bSpawn Whether to call spawn or not.
	*	@return Newly created entity, or null.
	*/
	static CBaseEntity* Create( const char* const pszClassName, const glm::vec3& vecOrigin, const glm::vec3& vecAngles, const bool bSpawn = true );

private:
	entity::Flags_t m_Flags = entity::FL_NONE;
	glm::vec3 m_vecOrigin;
	glm::vec3 m_vecAngles;

	float m_flTransparency = 1.0f;

public:
	entity::Flags_t GetFlags() const { return m_Flags; }
	bool AnyFlagsSet( const entity::Flags_t flags ) const { return ( m_Flags & flags ) != 0; }
	bool AllFlagsSet( const entity::Flags_t flags ) const { return ( m_Flags & flags ) == flags; }

	void InitFlags( const entity::Flags_t flags ) { m_Flags = flags; }
	void SetFlags( const entity::Flags_t flags ) { m_Flags |= flags; }
	void ClearFlags( const entity::Flags_t flags ) { m_Flags &= ~flags; }

	const glm::vec3& GetOrigin() const { return m_vecOrigin; }
	void SetOrigin( const glm::vec3& vecOrigin ) { m_vecOrigin = vecOrigin; }

	const glm::vec3& GetAngles() const { return m_vecAngles; }
	void SetAngles( const glm::vec3& vecAngles ) { m_vecAngles = vecAngles; }

	float GetTransparency() const { return m_flTransparency; }
	void SetTransparency( const float flTransparency ) { m_flTransparency = flTransparency; }

private:
	ThinkFunc_t m_ThinkFunc;
	float m_flLastThinkTime = 0;
	float m_flNextThinkTime = 0;

public:
	template<typename T>
	void SetThink( void ( T::*func )() )
	{
		m_ThinkFunc = static_cast<ThinkFunc_t>( func );
	}

	void SetThink( nullptr_t )
	{
		m_ThinkFunc = nullptr;
	}

	float GetLastThinkTime() const { return m_flLastThinkTime; }
	void SetLastThinkTime( const float flLastThink ) { m_flLastThinkTime = flLastThink; }

	float GetNextThinkTime() const { return m_flNextThinkTime; }
	void SetNextThinkTime( const float flNextThink ) { m_flNextThinkTime = flNextThink; }

	void Think()
	{
		if( m_ThinkFunc )
		{
			( this->*m_ThinkFunc )();
		}
	}

private:
	CBaseEntity( const CBaseEntity& ) = delete;
	CBaseEntity& operator=( const CBaseEntity& ) = delete;
};

#endif //GAME_CBASEENTITY_H