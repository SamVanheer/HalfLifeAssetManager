#include <algorithm>

#include <QMouseEvent>
#include <QOpenGLFunctions_1_1>
#include <QWheelEvent>

#include <glm/gtc/type_ptr.hpp>

#include "entity/BaseEntity.hpp"
#include "entity/EntityList.hpp"

#include "formats/studiomodel/IStudioModelRenderer.hpp"

#include "graphics/OpenGL.hpp"
#include "graphics/Scene.hpp"
#include "graphics/SceneContext.hpp"

#include "settings/ApplicationSettings.hpp"
#include "settings/ColorSettings.hpp"

#include "plugins/halflife/studiomodel/StudioModelAsset.hpp"
#include "plugins/halflife/studiomodel/StudioModelColors.hpp"
#include "application/EditorContext.hpp"

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

	SkyLight.Color = _entityContext->Asset->GetEditorContext()->GetColorSettings()->GetColor(studiomodel::SkyLightColor);
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
	auto applicationSettings = _entityContext->AppSettings;
	auto colors = _entityContext->Asset->GetEditorContext()->GetColorSettings();

	const auto backgroundColor = colors->GetColor(studiomodel::BackgroundColor);

	const float backgroundAlpha = applicationSettings->TransparentScreenshots ? 0 : 1;

	sc.OpenGLFunctions->glClearColor(backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundAlpha);
	sc.OpenGLFunctions->glClearStencil(0);
	sc.OpenGLFunctions->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	sc.OpenGLFunctions->glViewport(0, 0, _windowWidth, _windowHeight);

	sc.OpenGLFunctions->glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	_drawnPolygonsCount = 0;

	auto camera = GetCurrentCamera()->GetCamera();

	_entityContext->StudioModelRenderer->SetViewerOrigin(camera->GetOrigin());
	_entityContext->StudioModelRenderer->SetViewerRight(camera->GetRightVector());
	_entityContext->StudioModelRenderer->SetSkyLight(SkyLight);

	const unsigned int uiOldPolys = _entityContext->StudioModelRenderer->GetDrawnPolygonsCount();

	DrawRenderables(sc, RenderPass::Background);

	sc.OpenGLFunctions->glMatrixMode(GL_PROJECTION);
	sc.OpenGLFunctions->glLoadMatrixf(glm::value_ptr(camera->GetProjectionMatrix()));

	sc.OpenGLFunctions->glMatrixMode(GL_MODELVIEW);
	sc.OpenGLFunctions->glLoadMatrixf(glm::value_ptr(camera->GetViewMatrix()));

	DrawRenderables(sc, RenderPass::Standard);
	DrawRenderables(sc, RenderPass::Overlay3D);

	sc.OpenGLFunctions->glMatrixMode(GL_PROJECTION);
	sc.OpenGLFunctions->glLoadIdentity();

	sc.OpenGLFunctions->glOrtho(0.0f, (float)sc.WindowWidth, (float)sc.WindowHeight, 0.0f, 1.0f, -1.0f);

	sc.OpenGLFunctions->glMatrixMode(GL_MODELVIEW);
	sc.OpenGLFunctions->glLoadIdentity();

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

	const glm::vec3& cameraOrigin = _currentCamera->GetCamera()->GetOrigin();

	std::stable_sort(_renderablesToRender.begin(), _renderablesToRender.end(), [&](const auto& lhs, const auto& rhs)
		{
			const auto lhsDistance = lhs->GetRenderDistance(cameraOrigin);
			const auto rhsDistance = rhs->GetRenderDistance(cameraOrigin);

			// Farther objects render earlier.
			return lhsDistance > rhsDistance;
		});

	for (const auto& renderable : _renderablesToRender)
	{
		renderable->Draw(sc, renderPass);
	}
}
}
