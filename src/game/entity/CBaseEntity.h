#ifndef GAME_CBASEENTITY_H
#define GAME_CBASEENTITY_H

#include <glm/vec3.hpp>

#include "common/Class.h"

#include "CEntityDict.h"

#include "EntityConstants.h"

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
	virtual void Draw() {}

private:
	const char* m_pszClassName = nullptr;
	size_t m_uiEntIndex = entity::INVALID_ENTITY_INDEX;

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

	size_t GetEntIndex() const { return m_uiEntIndex; }
	void SetEntIndex( const size_t uiIndex ) { m_uiEntIndex = uiIndex; }

	static CBaseEntity* Create( const char* const pszClassName, const glm::vec3& vecOrigin, const glm::vec3& vecAngles, const bool bSpawn = true );

private:
	glm::vec3 m_vecOrigin;
	glm::vec3 m_vecAngles;

	float m_flTransparency = 1.0f;

public:
	const glm::vec3& GetOrigin() const { return m_vecOrigin; }
	void SetOrigin( const glm::vec3& vecOrigin ) { m_vecOrigin = vecOrigin; }

	const glm::vec3& GetAngles() const { return m_vecAngles; }
	void SetAngles( const glm::vec3& vecAngles ) { m_vecAngles = vecAngles; }

	float GetTransparency() const { return m_flTransparency; }
	void SetTransparency( const float flTransparency ) { m_flTransparency = flTransparency; }

private:
	ThinkFunc_t m_ThinkFunc;

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