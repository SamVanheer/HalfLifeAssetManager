#include <QMouseEvent>
#include <QWheelEvent>

#include <glm/gtc/type_ptr.hpp>

#include "entity/BaseEntity.hpp"
#include "entity/EntityList.hpp"

#include "formats/studiomodel/IStudioModelRenderer.hpp"

#include "graphics/OpenGL.hpp"
#include "graphics/Scene.hpp"
#include "graphics/SceneContext.hpp"

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

Scene::Scene(std::string&& name, EntityContext* entityContext)
	: _name(std::move(name))
	, _entityContext(entityContext)
	, _entityList(std::make_unique<EntityList>(_entityContext))
	, _defaultCameraOperator(std::make_unique<DefaultCameraOperator>())
{
	SetCurrentCamera(nullptr);
}

Scene::~Scene() = default;

void Scene::CreateDeviceObjects(SceneContext& sc)
{
	for (auto& entity : *_entityList)
	{
		entity->CreateDeviceObjects(sc);
	}
}

void Scene::DestroyDeviceObjects(SceneContext& sc)
{
	for (auto& entity : *_entityList)
	{
		entity->DestroyDeviceObjects(sc);
	}
}

void Scene::Tick()
{
	_entityList->RunFrame();
}

void Scene::Draw(SceneContext& sc)
{
	auto colors = _entityContext->Asset->GetEditorContext()->GetColorSettings();

	const auto backgroundColor = colors->GetColor(studiomodel::BackgroundColor.Name);

	sc.OpenGLFunctions->glClearColor(backgroundColor.r, backgroundColor.g, backgroundColor.b, 1.0f);
	sc.OpenGLFunctions->glClearStencil(0);
	sc.OpenGLFunctions->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	sc.OpenGLFunctions->glViewport(0, 0, _windowWidth, _windowHeight);

	sc.OpenGLFunctions->glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	_drawnPolygonsCount = 0;

	auto camera = GetCurrentCamera()->GetCamera();

	_entityContext->StudioModelRenderer->SetViewerOrigin(camera->GetOrigin());
	_entityContext->StudioModelRenderer->SetViewerRight(camera->GetRightVector());

	const unsigned int uiOldPolys = _entityContext->StudioModelRenderer->GetDrawnPolygonsCount();

	DrawRenderables(sc, RenderPass::Background);

	sc.OpenGLFunctions->glMatrixMode(GL_PROJECTION);
	sc.OpenGLFunctions->glLoadMatrixf(glm::value_ptr(camera->GetProjectionMatrix()));

	sc.OpenGLFunctions->glMatrixMode(GL_MODELVIEW);
	sc.OpenGLFunctions->glLoadMatrixf(glm::value_ptr(camera->GetViewMatrix()));

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

void Scene::DrawRenderables(SceneContext& sc, RenderPass::RenderPass renderPass)
{
	CollectRenderables(renderPass, _renderablesToRender);

	for (const auto& renderable : _renderablesToRender)
	{
		renderable->Draw(sc, renderPass);
	}
}
}
