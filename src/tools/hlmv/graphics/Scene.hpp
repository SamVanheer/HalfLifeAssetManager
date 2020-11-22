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

namespace soundsystem
{
class ISoundSystem;
}

namespace graphics
{
/**
*	@brief Contains all entities to be rendered for a particular scene
*/
class Scene
{
public:
	//TOOD: these need to be defined somewhere else
	static constexpr float DefaultFOV = 65.f;
	static constexpr float DefaultFirstPersonFieldOfView = 74.f;

	Scene(soundsystem::ISoundSystem* soundSystem);
	~Scene();
	Scene(const Scene&) = delete;
	Scene& operator=(const Scene&) = delete;

	EntityContext* GetEntityContext() const { return _entityContext.get(); }

	CCamera* GetCamera() { return &_camera; }

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

	unsigned int GetDrawnPolygonsCount() const { return _drawnPolygonsCount; }

	CHLMVStudioModelEntity* GetEntity() { return _entity; }

	void SetEntity(CHLMVStudioModelEntity* entity);

	void AlignOnGround();

	void Initialize();

	void Shutdown();

	void Tick();

	void Draw();

private:
	void ApplyCameraToScene();

	void SetupRenderMode(RenderMode renderMode = RenderMode::INVALID);

	void DrawModel();

	//TODO: these are temporary until the graphics code can be refactored into an object based design
public:
	RenderMode CurrentRenderMode = RenderMode::TEXTURE_SHADED;

	bool ShowHitboxes = false;
	bool ShowBones = false;
	bool ShowAttachments = false;
	bool ShowEyePosition = false;
	bool EnableBackfaceCulling = true;
	bool ShowGround = false;
	bool MirrorOnGround = false;
	bool ShowBackground = false;
	bool ShowWireframeOverlay = false;
	bool DrawShadows = false;
	bool FixShadowZFighting = false;
	bool ShowAxes = false;
	bool ShowNormals = false;
	bool ShowCrosshair = false;
	bool ShowGuidelines = false;
	bool ShowPlayerHitbox = false;

	float FieldOfView = DefaultFOV;
	float FirstPersonFieldOfView = DefaultFirstPersonFieldOfView;

	float* CurrentFOV = &FieldOfView;

	int FloorLength = 0;

	bool ShowTexture = false;

private:
	const std::unique_ptr<studiomdl::IStudioModelRenderer> _studioModelRenderer;

	std::unique_ptr<CWorldTime> _worldTime;

	std::unique_ptr<CEntityManager> _entityManager;

	std::unique_ptr<EntityContext> _entityContext;

	CCamera _camera;

	unsigned int _windowWidth = 0, _windowHeight = 0;

	glm::vec3 _backgroundColor{0.5f, 0.5f, 0.5f};

	unsigned int _drawnPolygonsCount = 0;

	CHLMVStudioModelEntity* _entity{};
};
}
