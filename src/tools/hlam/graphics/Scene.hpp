#pragma once

#include <memory>
#include <string>
#include <vector>

#include "graphics/Camera.hpp"
#include "graphics/GraphicsConstants.hpp"

class BaseEntity;
class EntityList;
class QOpenGLFunctions_1_1;
struct EntityContext;

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
	Scene(std::string&& name, IGraphicsContext* graphicsContext,QOpenGLFunctions_1_1* openglFunctions,
		graphics::TextureLoader* textureLoader, EntityContext* entityContext);
	~Scene();
	Scene(const Scene&) = delete;
	Scene& operator=(const Scene&) = delete;

	const std::string& GetName() const { return _name; }

	IGraphicsContext* GetGraphicsContext() { return _graphicsContext; }

	QOpenGLFunctions_1_1* GetOpenGLFunctions() { return _openglFunctions; }

	EntityList* GetEntityList() { return _entityList.get(); }

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
	const std::string _name;

	IGraphicsContext* const _graphicsContext;
	QOpenGLFunctions_1_1* const _openglFunctions;
	TextureLoader* const _textureLoader;
	EntityContext* const _entityContext;

	const std::unique_ptr<EntityList> _entityList;

	Camera* _currentCamera{};

	Camera _defaultCamera;

	unsigned int _windowWidth = 0, _windowHeight = 0;

	unsigned int _drawnPolygonsCount = 0;

	std::vector<BaseEntity*> _renderablesToRender;
};
}
