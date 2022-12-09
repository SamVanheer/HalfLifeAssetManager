#pragma once

#include <memory>
#include <vector>

#include "engine/shared/studiomodel/StudioModelFileFormat.hpp"

#include "graphics/Camera.hpp"
#include "graphics/GraphicsConstants.hpp"

class BaseEntity;
class EntityList;
class QOpenGLFunctions_1_1;
class WorldTime;
struct EntityContext;

namespace ui::assets::studiomodel
{
class StudioModelAsset;
}

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

// TODO: shouldn't depend on ui code here
namespace ui::settings
{
class StudioModelSettings;
}

namespace graphics
{
class IGraphicsContext;
class SceneContext;
class TextureLoader;

/**
*	@brief Contains all entities to be rendered for a particular scene
*/
class Scene
{
public:
	Scene(ui::assets::studiomodel::StudioModelAsset* asset,
		IGraphicsContext* graphicsContext, QOpenGLFunctions_1_1* openglFunctions,
		graphics::TextureLoader* textureLoader, soundsystem::ISoundSystem* soundSystem, WorldTime* worldTime,
		ui::settings::StudioModelSettings* settings);
	~Scene();
	Scene(const Scene&) = delete;
	Scene& operator=(const Scene&) = delete;

	IGraphicsContext* GetGraphicsContext() const { return _graphicsContext; }

	QOpenGLFunctions_1_1* GetOpenGLFunctions() { return _openglFunctions; }

	EntityContext* GetEntityContext() const { return _entityContext.get(); }

	EntityList* GetEntityList() const { return _entityList.get(); }

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

	unsigned int GetDrawnPolygonsCount() const { return _drawnPolygonsCount; }

	void CreateDeviceObjects();

	void DestroyDeviceObjects();

	void Tick();

	void Draw(SceneContext& sc);

private:
	void CollectRenderables(RenderPass::RenderPass renderPass, std::vector<BaseEntity*>& renderablesToRender);

	void DrawRenderables(graphics::SceneContext& sc, RenderPass::RenderPass renderPass);

private:
	IGraphicsContext* const _graphicsContext;

	QOpenGLFunctions_1_1* const _openglFunctions;

	TextureLoader* const _textureLoader;

	const std::unique_ptr<sprite::ISpriteRenderer> _spriteRenderer;
	const std::unique_ptr<studiomdl::IStudioModelRenderer> _studioModelRenderer;

	std::unique_ptr<EntityContext> _entityContext;

	std::unique_ptr<EntityList> _entityList;

	Camera* _currentCamera{};

	Camera _defaultCamera;

	unsigned int _windowWidth = 0, _windowHeight = 0;

	unsigned int _drawnPolygonsCount = 0;

	std::vector<BaseEntity*> _renderablesToRender;
};
}
