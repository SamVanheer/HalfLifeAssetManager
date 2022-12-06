#pragma once

#include <memory>

#include <GL/glew.h>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "engine/shared/studiomodel/StudioModelFileFormat.hpp"

#include "graphics/Camera.hpp"
#include "graphics/GraphicsConstants.hpp"

class EntityList;
class HLMVStudioModelEntity;
class StudioModelEntity;
class WorldTime;
struct EntityContext;

namespace sprite
{
class ISpriteRenderer;
}

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
class TextureLoader;

/**
*	@brief Contains all entities to be rendered for a particular scene
*/
class Scene
{
public:
	Scene(graphics::TextureLoader* textureLoader, soundsystem::ISoundSystem* soundSystem, WorldTime* worldTime);
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

		//Update the camera's projection matrix
		_currentCamera->SetWindowSize(_windowWidth, _windowHeight);
	}

	void UpdateWindowSize(unsigned int width, unsigned int height)
	{
		//Avoid constantly updating cameras
		if (_windowWidth != width || _windowHeight != height)
		{
			//TODO: recreate window sized resources
			_windowWidth = width;
			_windowHeight = height;

			//Always update the default camera
			_defaultCamera.SetWindowSize(_windowWidth, _windowHeight);

			if (_currentCamera != &_defaultCamera)
			{
				_currentCamera->SetWindowSize(_windowWidth, _windowHeight);
			}
		}
	}

	glm::vec3 GetLightColor() const;

	void SetLightColor(const glm::vec3& value);

	glm::vec3 GetWireframeColor() const;

	void SetWireframeColor(const glm::vec3& value);

	unsigned int GetDrawnPolygonsCount() const { return _drawnPolygonsCount; }

	HLMVStudioModelEntity* GetEntity() { return _entity; }

	void SetEntity(HLMVStudioModelEntity* entity)
	{
		_entity = entity;
	}

	void AlignOnGround();

	void Initialize();

	void Shutdown();

	void Tick();

	void Draw();

private:

	void DrawModel();

	//TODO: these are temporary until the graphics code can be refactored into an object based design
public:
	RenderMode CurrentRenderMode = RenderMode::TEXTURE_SHADED;

	bool ShowHitboxes = false;
	bool ShowBones = false;
	bool ShowAttachments = false;
	bool ShowEyePosition = false;
	bool ShowBBox = false;
	bool ShowCBox = false;
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
	bool EnableFloorTextureTiling{false};
	int FloorTextureLength{16};
	glm::vec3 FloorOrigin{0};

	GLuint GroundTexture{0};
	GLuint BackgroundTexture{0};

	int DrawSingleBoneIndex = -1;
	int DrawSingleAttachmentIndex = -1;
	int DrawSingleHitboxIndex = -1;

	bool CameraIsFirstPerson{false};

	//TODO: having some colors as variables and some as methods is inconsistent
	glm::vec3 GroundColor{0};
	glm::vec3 BackgroundColor{0.5};
	glm::vec3 CrosshairColor{1};

private:
	//Keep track of how many times we've been initialized and shut down so we don't do it at the wrong time
	int _initializeCount{0};

	TextureLoader* const _textureLoader;

	std::unique_ptr<IGraphicsContext> _graphicsContext;

	const std::unique_ptr<sprite::ISpriteRenderer> _spriteRenderer;
	const std::unique_ptr<studiomdl::IStudioModelRenderer> _studioModelRenderer;

	WorldTime* const _worldTime;

	std::unique_ptr<EntityList> _entityList;

	std::unique_ptr<EntityContext> _entityContext;

	Camera* _currentCamera{};

	Camera _defaultCamera;

	unsigned int _windowWidth = 0, _windowHeight = 0;

	unsigned int _drawnPolygonsCount = 0;

	HLMVStudioModelEntity* _entity{};

	int _floorSequence{-1};
	float _previousFloorFrame{0};

	glm::vec2 _floorTextureOffset{0};
};
}
