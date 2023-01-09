#pragma once

#include <memory>
#include <string>
#include <vector>

#include "graphics/Camera.hpp"
#include "graphics/GraphicsConstants.hpp"
#include "graphics/Light.hpp"

class BaseEntity;
class EntityList;
class EntityContext;

namespace graphics
{
class SceneContext;
class TextureLoader;

/**
*	@brief Contains all entities to be rendered for a particular scene
*/
class Scene
{
public:
	Scene(std::string&& name, EntityContext* entityContext);
	~Scene();
	Scene(const Scene&) = delete;
	Scene& operator=(const Scene&) = delete;

	const std::string& GetName() const { return _name; }

	EntityList* GetEntityList() { return _entityList.get(); }

	ICameraOperator* GetCurrentCamera() { return _currentCamera; }

	void SetCurrentCamera(ICameraOperator* camera)
	{
		if (!camera)
		{
			camera = _defaultCameraOperator.get();
		}

		_currentCamera = camera;

		//Update the camera's projection matrix
		_currentCamera->GetCamera()->SetWindowSize(_windowWidth, _windowHeight);
	}

	void UpdateWindowSize(unsigned int width, unsigned int height)
	{
		//Avoid constantly updating cameras
		if (_windowWidth != width || _windowHeight != height)
		{
			//TODO: recreate window sized resources
			_windowWidth = width;
			_windowHeight = height;

			_currentCamera->GetCamera()->SetWindowSize(_windowWidth, _windowHeight);
		}
	}

	unsigned int GetDrawnPolygonsCount() const { return _drawnPolygonsCount; }

	void CreateDeviceObjects(SceneContext& sc);

	void DestroyDeviceObjects(SceneContext& sc);

	void Tick();

	void Draw(SceneContext& sc);

private:
	void CollectRenderables(RenderPass::RenderPass renderPass, std::vector<BaseEntity*>& renderablesToRender);

	void DrawRenderables(SceneContext& sc, RenderPass::RenderPass renderPass);

public:
	Light SkyLight;

private:
	const std::string _name;

	EntityContext* const _entityContext;

	const std::unique_ptr<EntityList> _entityList;

	const std::unique_ptr<ICameraOperator> _defaultCameraOperator;

	ICameraOperator* _currentCamera{};

	unsigned int _windowWidth = 0, _windowHeight = 0;

	unsigned int _drawnPolygonsCount = 0;

	std::vector<BaseEntity*> _renderablesToRender;
};
}
