#pragma once

#include <memory>

class CBaseEntity;
class CBaseEntityList;
class CWorldTime;

/**
*	Manages entities.
*/
class CEntityManager final
{
public:
	CEntityManager(std::unique_ptr<CBaseEntityList>&& entityList, CWorldTime* worldTime);
	~CEntityManager();

	CBaseEntityList* GetEntityList() const { return _entityList.get(); }

	/**
	*	Called on startup. Initializes the manager.
	*	@return true on success, false otherwise.
	*/
	bool Initialize();

	/**
	*	Called on shutdown.
	*/
	void Shutdown();

	/**
	*	Returns whether a map is running.
	*/
	bool IsMapRunning() const { return m_bMapRunning; }

	/**
	*	Called when a map begins.
	*/
	bool OnMapBegin();

	/**
	*	Called when a map ends. Removes all entities.
	*/
	void OnMapEnd();

	/**
	*	Runs a single frame for all entities. Removes entities flagged as needing removal.
	*/
	void RunFrame();

	/**
	*	Creates a new entity by classname. This is the one place where entities can be created
	*	@param pszClassName The entity's class name
	*	@param context Entity context
	*	@param vecOrigin The entity's origin
	*	@param vecAngles The entity's angles
	*	@param bSpawn Whether to call spawn
	*	@return Newly created entity, or null
	*/
	CBaseEntity* Create(const char* const pszClassName, EntityContext* context,
		const glm::vec3& vecOrigin, const glm::vec3& vecAngles, const bool bSpawn = true);

private:
	std::unique_ptr<CBaseEntityList> _entityList;
	CWorldTime* const _worldTime;

	bool m_bMapRunning = false;

private:
	CEntityManager( const CEntityManager& ) = delete;
	CEntityManager& operator=( const CEntityManager& ) = delete;
};
