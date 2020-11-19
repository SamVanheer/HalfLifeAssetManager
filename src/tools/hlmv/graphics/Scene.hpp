#pragma once

#include <memory>

#include <glm/vec3.hpp>

#include "graphics/CCamera.h"

#include "graphics/Constants.h"

class CEntityManager;
class CHLMVStudioModelEntity;
class CWorldTime;
struct EntityContext;

namespace studiomdl
{
class IStudioModelRenderer;
}

namespace graphics
{
/**
*	@brief Contains all entities to be rendered for a particular scene
*/
class Scene
{
public:
	Scene();
	~Scene();
	Scene(const Scene&) = delete;
	Scene& operator=(const Scene&) = delete;

	EntityContext* GetEntityContext() const { return _entityContext.get(); }

	void UpdateWindowSize(unsigned int width, unsigned int height)
	{
		//TODO: recreate window sized resources
		_windowWidth = width;
		_windowHeight = height;
	}

	glm::vec3 GetBackgroundColor() const { return _backgroundColor; }

	void SetBackgroundColor(const glm::vec3& value)
	{
		_backgroundColor = value;
	}

	bool ShouldMirrorOnGround() const { return _mirrorOnGround; }

	void SetMirrorOnGround(bool value)
	{
		_mirrorOnGround = value;
	}

	bool ShouldShowTexture() const { return _showTexture; }

	void SetShowTexture(bool value)
	{
		_showTexture = value;
	}

	unsigned int GetDrawnPolygonsCount() const { return _drawnPolygonsCount; }

	CHLMVStudioModelEntity* GetEntity() { return _entity; }

	void SetEntity(CHLMVStudioModelEntity* entity);

	void Initialize();

	void Shutdown();

	void Tick();

	void Draw();

private:
	void ApplyCameraToScene();

	void SetupRenderMode(RenderMode renderMode = RenderMode::INVALID);

	void DrawModel();

private:
	const std::unique_ptr<studiomdl::IStudioModelRenderer> _studioModelRenderer;

	std::unique_ptr<CWorldTime> _worldTime;

	std::unique_ptr<CEntityManager> _entityManager;

	std::unique_ptr<EntityContext> _entityContext;

	unsigned int _windowWidth = 0, _windowHeight = 0;

	glm::vec3 _backgroundColor{0.5f, 0.5f, 0.5f};

	bool _mirrorOnGround = false;
	bool _showTexture = false;

	unsigned int _drawnPolygonsCount = 0;

	CCamera _camera;

	CHLMVStudioModelEntity* _entity{};
};
}
