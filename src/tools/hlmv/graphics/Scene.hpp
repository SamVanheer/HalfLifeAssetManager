#pragma once

#include <memory>

#include <GL/glew.h>

#include <glm/vec3.hpp>

#include "engine/shared/studiomodel/studio.h"

#include "graphics/Camera.hpp"

#include "graphics/Constants.h"

class CEntityManager;
class CHLMVStudioModelEntity;
class CStudioModelEntity;
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
class IGraphicsContext;

/**
*	@brief Contains all entities to be rendered for a particular scene
*/
class Scene
{
public:
	Scene(soundsystem::ISoundSystem* soundSystem, CWorldTime* worldTime);
	~Scene();
	Scene(const Scene&) = delete;
	Scene& operator=(const Scene&) = delete;

	IGraphicsContext* GetGraphicsContext() const { return _graphicsContext.get(); }

	void SetGraphicsContext(std::unique_ptr<IGraphicsContext>&& graphicsContext);

	EntityContext* GetEntityContext() const { return _entityContext.get(); }

	Camera* GetCurrentCamera() { return _currentCamera; }

	void SetCurrentCamera(Camera* camera)
	{
		if (!camera)
		{
			camera = &_defaultCamera;
		}

		_currentCamera = camera;
	}

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

	void SetEntity(CHLMVStudioModelEntity* entity)
	{
		_entity = entity;
	}

	void AlignOnGround();

	void Initialize();

	void Shutdown();

	void Tick();

	void Draw();

private:
	void ApplyCameraToScene();

	void SetupRenderMode(RenderMode renderMode = RenderMode::INVALID);

	void DrawModel();

	void DrawTexture(const int xOffset, const int yOffset, const int width, const int height, CStudioModelEntity* entity,
		const int textureIndex, const float textureScale, const bool showUVMap, const bool overlayUVMap);

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

	int FloorLength = 0;

	GLuint GroundTexture{0};
	GLuint BackgroundTexture{0};

	int DrawSingleBoneIndex = -1;
	int DrawSingleAttachmentIndex = -1;
	int DrawSingleHitboxIndex = -1;

	bool ShowTexture = false;

	int TextureIndex{};

	int TextureXOffset{}, TextureYOffset{};

	float TextureScale{1.f};

	bool ShowUVMap{};
	bool OverlayUVMap{};

	GLuint UVMeshTexture{0};

private:
	//Keep track of how many times we've been initialized and shut down so we don't do it at the wrong time
	int _initializeCount{0};

	std::unique_ptr<IGraphicsContext> _graphicsContext;

	const std::unique_ptr<studiomdl::IStudioModelRenderer> _studioModelRenderer;

	CWorldTime* const _worldTime;

	std::unique_ptr<CEntityManager> _entityManager;

	std::unique_ptr<EntityContext> _entityContext;

	Camera* _currentCamera{};

	Camera _defaultCamera;

	unsigned int _windowWidth = 0, _windowHeight = 0;

	glm::vec3 _backgroundColor{0.5f, 0.5f, 0.5f};

	unsigned int _drawnPolygonsCount = 0;

	CHLMVStudioModelEntity* _entity{};
};
}
