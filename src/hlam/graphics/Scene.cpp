#include <QMouseEvent>
#include <QWheelEvent>

#include <glm/gtc/type_ptr.hpp>

#include "entity/BaseEntity.hpp"
#include "entity/EntityList.hpp"

#include "formats/studiomodel/IStudioModelRenderer.hpp"

#include "graphics/OpenGL.hpp"
#include "graphics/Scene.hpp"

#include "ui/assets/studiomodel/StudioModelAsset.hpp"
#include "ui/assets/studiomodel/StudioModelColors.hpp"
#include "ui/EditorContext.hpp"
#include "ui/settings/ColorSettings.hpp"

namespace graphics
{
namespace
{
class DefaultCameraOperator final : public ICameraOperator
{
public:
	Camera* GetCamera() override { return &_camera; }

	void MouseEvent(QMouseEvent& event) override
	{
		event.ignore();
	}

	void WheelEvent(QWheelEvent& event) override
	{
		event.ignore();
	}

private:
	Camera _camera;
};
}

Scene::Scene(std::string&& name, IGraphicsContext* graphicsContext, QOpenGLFunctions_1_1* openglFunctions,
	TextureLoader* textureLoader, EntityContext* entityContext)
	: _name(std::move(name))
	, _graphicsContext(graphicsContext)
	, _openglFunctions(openglFunctions)
	, _textureLoader(textureLoader)
	, _entityContext(entityContext)
	, _entityList(std::make_unique<EntityList>(_entityContext))
	, _defaultCameraOperator(std::make_unique<DefaultCameraOperator>())
{
	SetCurrentCamera(nullptr);
}

Scene::~Scene() = default;

void Scene::CreateDeviceObjects()
{
	for (auto& entity : *_entityList)
	{
		entity->CreateDeviceObjects(_openglFunctions, *_textureLoader);
	}
}

void Scene::DestroyDeviceObjects()
{
	for (auto& entity : *_entityList)
	{
		entity->DestroyDeviceObjects(_openglFunctions, *_textureLoader);
	}
}

void Scene::Tick()
{
	_entityList->RunFrame();
}

void Scene::Draw(SceneContext& sc)
{
	auto colors = _entityContext->Asset->GetEditorContext()->GetColorSettings();

	const auto backgroundColor = ui::assets::studiomodel::ColorToVector(colors->GetColor(ui::assets::studiomodel::BackgroundColor.Name));

	_openglFunctions->glClearColor(backgroundColor.r, backgroundColor.g, backgroundColor.b, 1.0f);
	_openglFunctions->glClearStencil(0);
	_openglFunctions->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	_openglFunctions->glViewport(0, 0, _windowWidth, _windowHeight);

	_openglFunctions->glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	_drawnPolygonsCount = 0;

	auto camera = GetCurrentCamera()->GetCamera();

	_entityContext->StudioModelRenderer->SetViewerOrigin(camera->GetOrigin());
	_entityContext->StudioModelRenderer->SetViewerRight(camera->GetRightVector());

	const unsigned int uiOldPolys = _entityContext->StudioModelRenderer->GetDrawnPolygonsCount();

	DrawRenderables(sc, RenderPass::Background);

	_openglFunctions->glMatrixMode(GL_PROJECTION);
	_openglFunctions->glLoadMatrixf(glm::value_ptr(camera->GetProjectionMatrix()));

	_openglFunctions->glMatrixMode(GL_MODELVIEW);
	_openglFunctions->glLoadMatrixf(glm::value_ptr(camera->GetViewMatrix()));

	DrawRenderables(sc, RenderPass::Standard);
	DrawRenderables(sc, RenderPass::Overlay3D);
	DrawRenderables(sc, RenderPass::Overlay2D);

	_drawnPolygonsCount = _entityContext->StudioModelRenderer->GetDrawnPolygonsCount() - uiOldPolys;
}

void Scene::CollectRenderables(RenderPass::RenderPass renderPass, std::vector<BaseEntity*>& renderablesToRender)
{
	renderablesToRender.clear();

	for (auto& entity : *_entityList)
	{
		if (entity->GetRenderPasses() & renderPass)
		{
			renderablesToRender.emplace_back(entity.get());
		}
	}
}

void Scene::DrawRenderables(graphics::SceneContext& sc, RenderPass::RenderPass renderPass)
{
	CollectRenderables(renderPass, _renderablesToRender);

	for (const auto& renderable : _renderablesToRender)
	{
		renderable->Draw(_openglFunctions, sc, renderPass);
	}
}
}
