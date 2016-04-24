#ifndef GAME_ENTITY_CENTITYMANAGER_H
#define GAME_ENTITY_CENTITYMANAGER_H

/**
*	Manages entities.
*/
class CEntityManager final
{
public:
	CEntityManager();
	~CEntityManager();

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

private:
	bool m_bMapRunning = false;

private:
	CEntityManager( const CEntityManager& ) = delete;
	CEntityManager& operator=( const CEntityManager& ) = delete;
};

CEntityManager& EntityManager();

#endif //GAME_ENTITY_CENTITYMANAGER_H