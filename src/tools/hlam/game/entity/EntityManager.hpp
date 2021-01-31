#pragma once

#include <memory>

class BaseEntity;
class BaseEntityList;
class WorldTime;

/**
*	Manages entities.
*/
class EntityManager final
{
public:
	EntityManager(std::unique_ptr<BaseEntityList>&& entityList, WorldTime* worldTime);
	~EntityManager();

	EntityManager(const EntityManager&) = delete;
	EntityManager& operator=(const EntityManager&) = delete;

	BaseEntityList* GetEntityList() const { return _entityList.get(); }

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
	bool IsMapRunning() const { return _mapRunning; }

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
	*	@param origin The entity's origin
	*	@param angles The entity's angles
	*	@param bSpawn Whether to call spawn
	*	@return Newly created entity, or null
	*/
	BaseEntity* Create(const char* const pszClassName, EntityContext* context,
		const glm::vec3& origin, const glm::vec3& angles, const bool bSpawn = true);

private:
	std::unique_ptr<BaseEntityList> _entityList;
	WorldTime* const _worldTime;

	bool _mapRunning = false;
};
