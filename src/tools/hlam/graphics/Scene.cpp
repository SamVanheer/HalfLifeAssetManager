#include <cassert>
#include <utility>

#include <qopenglfunctions_1_1.h>

#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "engine/renderer/sprite/SpriteRenderer.hpp"
#include "engine/renderer/studiomodel/StudioModelRenderer.hpp"
#include "engine/shared/renderer/studiomodel/IStudioModelRenderer.hpp"

#include "entity/BaseEntity.hpp"
#include "entity/EntityList.hpp"

#include "graphics/IGraphicsContext.hpp"
#include "graphics/Scene.hpp"
#include "graphics/TextureLoader.hpp"

#include "qt/QtLogSink.hpp"

#include "ui/assets/studiomodel/StudioModelAsset.hpp"
#include "ui/assets/studiomodel/StudioModelColors.hpp"
#include "ui/EditorContext.hpp"
#include "ui/settings/ColorSettings.hpp"

namespace graphics
{
Scene::Scene(ui::assets::studiomodel::StudioModelAsset* asset,
	TextureLoader* textureLoader, soundsystem::ISoundSystem* soundSystem, WorldTime* worldTime,
	ui::settings::StudioModelSettings* settings)
	: _textureLoader(textureLoader)
	, _spriteRenderer(std::make_unique<sprite::SpriteRenderer>(CreateQtLoggerSt(logging::HLAMSpriteRenderer()), worldTime))
	, _studioModelRenderer(std::make_unique<studiomdl::StudioModelRenderer>(CreateQtLoggerSt(logging::HLAMStudioModelRenderer()), asset->GetEditorContext()->GetColorSettings()))
	, _entityContext(std::make_unique<EntityContext>(asset,
		worldTime,
		_studioModelRenderer.get(), _spriteRenderer.get(),
		soundSystem,
		settings,
		this))
	, _entityList(std::make_unique<EntityList>(_entityContext.get()))
{
	assert(_textureLoader);

	SetCurrentCamera(nullptr);
}

Scene::~Scene()
{
	_entityList->DestroyAll();
}

void Scene::SetGraphicsContext(std::unique_ptr<IGraphicsContext>&& graphicsContext)
{
	_graphicsContext = std::move(graphicsContext);
}

void Scene::SetOpenGLFunctions(QOpenGLFunctions_1_1* openglFunctions)
{
	_openglFunctions = openglFunctions;
	_textureLoader->SetOpenGLFunctions(openglFunctions);
}

void Scene::Initialize()
{
	++_initializeCount;

	if (_initializeCount != 1)
	{
		return;
	}

	if (!_studioModelRenderer->Initialize())
	{
		//TODO: handle error
	}

	for (auto& entity : *_entityList)
	{
		entity->CreateDeviceObjects(_openglFunctions, *_textureLoader);
	}
}

void Scene::Shutdown()
{
	--_initializeCount;

	if (_initializeCount != 0)
	{
		return;
	}

	for (auto& entity : *_entityList)
	{
		entity->DestroyDeviceObjects(_openglFunctions, *_textureLoader);
	}

	_studioModelRenderer->Shutdown();
}

void Scene::Tick()
{
	_studioModelRenderer->RunFrame();
	_entityList->RunFrame();
}

void Scene::Draw()
{
	auto colors = _entityContext->Asset->GetEditorContext()->GetColorSettings();

	const auto backgroundColor = ui::assets::studiomodel::ColorToVector(colors->GetColor(ui::assets::studiomodel::BackgroundColor.Name));

	_openglFunctions->glClearColor(backgroundColor.r, backgroundColor.g, backgroundColor.b, 1.0f);
	_openglFunctions->glClearStencil(0);
	_openglFunctions->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	_openglFunctions->glViewport(0, 0, _windowWidth, _windowHeight);

	_openglFunctions->glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	_drawnPolygonsCount = 0;

	auto camera = GetCurrentCamera();

	_studioModelRenderer->SetOpenGLFunctions(_openglFunctions);
	_studioModelRenderer->SetViewerOrigin(camera->GetOrigin());
	_studioModelRenderer->SetViewerRight(camera->GetRightVector());

	const unsigned int uiOldPolys = _studioModelRenderer->GetDrawnPolygonsCount();

	DrawRenderables(RenderPass::Background);

	_openglFunctions->glMatrixMode(GL_PROJECTION);
	_openglFunctions->glLoadMatrixf(glm::value_ptr(camera->GetProjectionMatrix()));

	_openglFunctions->glMatrixMode(GL_MODELVIEW);
	_openglFunctions->glLoadMatrixf(glm::value_ptr(camera->GetViewMatrix()));

	DrawRenderables(RenderPass::Standard);
	DrawRenderables(RenderPass::Overlay3D);
	DrawRenderables(RenderPass::Overlay2D);

	_drawnPolygonsCount = _studioModelRenderer->GetDrawnPolygonsCount() - uiOldPolys;
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

void Scene::DrawRenderables(RenderPass::RenderPass renderPass)
{
	CollectRenderables(renderPass, _renderablesToRender);

	for (const auto& renderable : _renderablesToRender)
	{
		renderable->Draw(_openglFunctions, renderPass);
	}
}
}
