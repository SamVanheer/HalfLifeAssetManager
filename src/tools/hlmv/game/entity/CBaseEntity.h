#ifndef GAME_CBASEENTITY_H
#define GAME_CBASEENTITY_H

#include <glm/vec3.hpp>

#include "shared/Class.h"

#include "engine/shared/renderer/DrawConstants.h"

#include "EntityConstants.h"
#include "EHandle.h"
#include "CEntityDict.h"

//Windows defines this
#ifdef GetClassName
#undef GetClassName
#endif

namespace studiomdl
{
class IStudioModelRenderer;
}

class CBaseEntity;
class CBaseEntityList;
class CEntityManager;
class CWorldTime;

/**
*	Pointer to member function used for think methods.
*/
using ThinkFunc_t = void ( CBaseEntity::* )();

struct EntityContext final
{
	//TODO: improve this
	CWorldTime* const WorldTime;
	studiomdl::IStudioModelRenderer* const StudioModelRenderer;
	CBaseEntityList* const EntityList;
	CEntityManager* const EntityManager;

	EntityContext(CWorldTime* worldTime, studiomdl::IStudioModelRenderer* studioModelRenderer, CBaseEntityList* entityList, CEntityManager* entityManager)
		: WorldTime(worldTime)
		, StudioModelRenderer(studioModelRenderer)
		, EntityList(entityList)
		, EntityManager(entityManager)
	{
	}
};

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
	virtual void Draw( renderer::DrawFlags_t flags ) {}

private:
	const char* m_pszClassName = nullptr;
	EHandle m_EntHandle;

	EntityContext* _context{};

public:
	/**
	*	Called when the entity is constructed. Do not call directly.
	*	@param pszClassName The entity's class name
	*	@param context Entity context
	*/
	void Construct(const char* const pszClassName, EntityContext* context);

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

	EntityContext* GetContext() const { return _context; }

private:
	entity::Flags_t m_Flags = entity::FL_NONE;
	glm::vec3 m_vecOrigin;
	glm::vec3 m_vecAngles;
	glm::vec3 m_vecScale = { 1.0f, 1.0f, 1.0f };

	float m_flTransparency = 1.0f;

public:
	/**
	*	Gets the entity's flags.
	*/
	entity::Flags_t GetFlags() const { return m_Flags; }

	/**
	*	Returns if any of the given flags are set.
	*/
	bool AnyFlagsSet( const entity::Flags_t flags ) const { return ( m_Flags & flags ) != 0; }

	/**
	*	Returns if all of the given flags are set.
	*/
	bool AllFlagsSet( const entity::Flags_t flags ) const { return ( m_Flags & flags ) == flags; }

	/**
	*	Sets the entity's flags to the given flags.
	*/
	void InitFlags( const entity::Flags_t flags ) { m_Flags = flags; }

	/**
	*	Sets the given flags on the entity. Existing flags are unaffected.
	*/
	void SetFlags( const entity::Flags_t flags ) { m_Flags |= flags; }

	/**
	*	Clears the given flags from the entity's flags.
	*/
	void ClearFlags( const entity::Flags_t flags ) { m_Flags &= ~flags; }

	/**
	*	Gets the entity's origin.
	*/
	const glm::vec3& GetOrigin() const { return m_vecOrigin; }

	/**
	*	@copydoc GetOrigin() const
	*	The returned vector can be modified and affects the origin.
	*/
	glm::vec3& GetOrigin() { return m_vecOrigin; }

	/**
	*	Sets the entity's origin.
	*/
	void SetOrigin( const glm::vec3& vecOrigin ) { m_vecOrigin = vecOrigin; }

	/**
	*	Gets the entity's angles.
	*/
	const glm::vec3& GetAngles() const { return m_vecAngles; }

	/**
	*	@copydoc GetAngles() const
	*	The returned vector can be modified and affects the angles.
	*/
	glm::vec3& GetAngles() { return m_vecAngles; }

	/**
	*	Sets the entity's angles.
	*/
	void SetAngles( const glm::vec3& vecAngles ) { m_vecAngles = vecAngles; }

	/**
	*	Gets the entity's scale.
	*/
	const glm::vec3& GetScale() const { return m_vecScale; }

	/**
	*	@copydoc GetScale() const
	*	The returned vector can be modified and affects the scale.
	*/
	glm::vec3& GetScale() { return m_vecScale; }

	/**
	*	Sets the entity's scale.
	*/
	void SetScale( const glm::vec3& vecScale ) { m_vecScale = vecScale; }

	/**
	*	Gets the entity's transparency.
	*/
	float GetTransparency() const { return m_flTransparency; }

	/**
	*	Sets the entity's transparency. TODO: clamp to [0, 1]
	*/
	void SetTransparency( const float flTransparency ) { m_flTransparency = flTransparency; }

private:
	ThinkFunc_t m_ThinkFunc = nullptr;
	float m_flLastThinkTime = 0;
	float m_flNextThinkTime = 0;

public:
	/**
	*	Gets the think method.
	*/
	ThinkFunc_t GetThink() const { return m_ThinkFunc; }

	/**
	*	Sets the entity's think method.
	*/
	template<typename T>
	void SetThink( void ( T::*func )() )
	{
		m_ThinkFunc = static_cast<ThinkFunc_t>( func );
	}

	/**
	*	@see SetThink( void ( T::*func )() )
	*	Overload used to set the think method to null.
	*/
	void SetThink( nullptr_t )
	{
		m_ThinkFunc = nullptr;
	}

	/**
	*	Gets the last think time.
	*/
	float GetLastThinkTime() const { return m_flLastThinkTime; }

	/**
	*	Sets the last think time. Should only be used by the entity manager.
	*/
	void SetLastThinkTime( const float flLastThink ) { m_flLastThinkTime = flLastThink; }

	/**
	*	Gets the next think time.
	*/
	float GetNextThinkTime() const { return m_flNextThinkTime; }

	/**
	*	Sets the next think time.
	*/
	void SetNextThinkTime( const float flNextThink ) { m_flNextThinkTime = flNextThink; }

	/**
	*	Runs the think method. NOTE: non-virtual.
	*/
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