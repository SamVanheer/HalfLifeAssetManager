#include <cassert>
#include <cmath>
#include <utility>

#include <qopenglfunctions_1_1.h>

#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>

#include "engine/renderer/sprite/SpriteRenderer.hpp"
#include "engine/renderer/studiomodel/StudioModelRenderer.hpp"
#include "engine/shared/renderer/studiomodel/IStudioModelRenderer.hpp"

#include "entity/EntityList.hpp"
#include "entity/HLMVStudioModelEntity.hpp"

#include "graphics/GraphicsUtils.hpp"
#include "graphics/IGraphicsContext.hpp"
#include "graphics/Scene.hpp"
#include "graphics/TextureLoader.hpp"

#include "qt/QtLogSink.hpp"

#include "ui/settings/StudioModelSettings.hpp"

#include "utility/WorldTime.hpp"

namespace graphics
{
static const int CROSSHAIR_LINE_WIDTH = 3;
static const int CROSSHAIR_LINE_START = 5;
static const int CROSSHAIR_LINE_LENGTH = 10;
static const int CROSSHAIR_LINE_END = CROSSHAIR_LINE_START + CROSSHAIR_LINE_LENGTH;

static const int GUIDELINES_LINE_WIDTH = 1;
static const int GUIDELINES_LINE_LENGTH = 5;
static const int GUIDELINES_POINT_LINE_OFFSET = 2;
static const int GUIDELINES_OFFSET = GUIDELINES_LINE_LENGTH + (GUIDELINES_POINT_LINE_OFFSET * 2) + GUIDELINES_LINE_WIDTH;

static const int GUIDELINES_EDGE_WIDTH = 4;

Scene::Scene(TextureLoader* textureLoader, soundsystem::ISoundSystem* soundSystem, WorldTime* worldTime, ui::settings::StudioModelSettings* settings)
	: _textureLoader(textureLoader)
	, _spriteRenderer(std::make_unique<sprite::SpriteRenderer>(CreateQtLoggerSt(logging::HLAMSpriteRenderer()), worldTime))
	, _studioModelRenderer(std::make_unique<studiomdl::StudioModelRenderer>(CreateQtLoggerSt(logging::HLAMStudioModelRenderer())))
	, _worldTime(worldTime)
	, _entityList(std::make_unique<EntityList>(_worldTime))
	, _entityContext(std::make_unique<EntityContext>(_worldTime,
		_studioModelRenderer.get(), _spriteRenderer.get(),
		_entityList.get(),
		soundSystem,
		settings))
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

glm::vec3 Scene::GetLightColor() const
{
	return _studioModelRenderer->GetLightColor();
}

void Scene::SetLightColor(const glm::vec3& value)
{
	_studioModelRenderer->SetLightColor(value);
}

glm::vec3 Scene::GetWireframeColor() const
{
	return _studioModelRenderer->GetWireframeColor();
}

void Scene::SetWireframeColor(const glm::vec3& value)
{
	_studioModelRenderer->SetWireframeColor(value);
}

void Scene::AlignOnGround()
{
	auto entity = GetEntity();

	auto model = entity->GetEditableModel();

	//First try finding the idle sequence, since that typically represents a model "at rest"
	//Failing that, use the first sequence
	auto idleFinder = [&]() -> const studiomdl::Sequence*
	{
		if (model->Sequences.empty())
		{
			return nullptr;
		}

		for (const auto& sequence : model->Sequences)
		{
			if (sequence->Label == "idle")
			{
				return sequence.get();
			}
		}

		return model->Sequences[0].get();
	};

	auto sequence = idleFinder();

	entity->SetOrigin({0, 0, sequence ? -sequence->BBMin.z : 0});
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
}

void Scene::Shutdown()
{
	--_initializeCount;

	if (_initializeCount != 0)
	{
		return;
	}

	_studioModelRenderer->Shutdown();

	if (BackgroundTexture != 0)
	{
		_openglFunctions->glDeleteTextures(1, &BackgroundTexture);
		BackgroundTexture = 0;
	}

	if (GroundTexture != 0)
	{
		_openglFunctions->glDeleteTextures(1, &GroundTexture);
		GroundTexture = 0;
	}
}

void Scene::Tick()
{
	_entityList->RunFrame();
}

void Scene::Draw()
{
	//TODO: really ugly, needs reworking
	if (nullptr != _entity)
	{
		auto model = _entity->GetEditableModel();
		
		if (model->TexturesNeedCreating)
		{
			model->TexturesNeedCreating = false;
			model->CreateTextures(*_textureLoader);
		}
	}

	_openglFunctions->glClearColor(BackgroundColor.r, BackgroundColor.g, BackgroundColor.b, 1.0f);

	if (_entityContext->Settings->MirrorOnGround)
	{
		_openglFunctions->glClearStencil(0);

		_openglFunctions->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	}
	else
		_openglFunctions->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	_openglFunctions->glViewport(0, 0, _windowWidth, _windowHeight);

	_openglFunctions->glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	_drawnPolygonsCount = 0;

	DrawModel();

	const int centerX = _windowWidth / 2;
	const int centerY = _windowHeight / 2;

	if (_entityContext->Settings->ShowCrosshair)
	{
		_openglFunctions->glMatrixMode(GL_PROJECTION);
		_openglFunctions->glLoadIdentity();

		_openglFunctions->glOrtho(0.0f, (float)_windowWidth, (float)_windowHeight, 0.0f, 1.0f, -1.0f);

		_openglFunctions->glMatrixMode(GL_MODELVIEW);
		_openglFunctions->glPushMatrix();
		_openglFunctions->glLoadIdentity();

		_openglFunctions->glDisable(GL_CULL_FACE);

		_openglFunctions->glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		_openglFunctions->glDisable(GL_TEXTURE_2D);

		_openglFunctions->glColor4fv(glm::value_ptr(glm::vec4{CrosshairColor, 1}));

		_openglFunctions->glPointSize(CROSSHAIR_LINE_WIDTH);
		_openglFunctions->glLineWidth(CROSSHAIR_LINE_WIDTH);

		_openglFunctions->glBegin(GL_POINTS);

		_openglFunctions->glVertex2f(centerX - CROSSHAIR_LINE_WIDTH / 2, centerY + 1);

		_openglFunctions->glEnd();

		_openglFunctions->glBegin(GL_LINES);

		_openglFunctions->glVertex2f(centerX - CROSSHAIR_LINE_START, centerY);
		_openglFunctions->glVertex2f(centerX - CROSSHAIR_LINE_END, centerY);

		_openglFunctions->glVertex2f(centerX + CROSSHAIR_LINE_START, centerY);
		_openglFunctions->glVertex2f(centerX + CROSSHAIR_LINE_END, centerY);

		_openglFunctions->glVertex2f(centerX, centerY - CROSSHAIR_LINE_START);
		_openglFunctions->glVertex2f(centerX, centerY - CROSSHAIR_LINE_END);

		_openglFunctions->glVertex2f(centerX, centerY + CROSSHAIR_LINE_START);
		_openglFunctions->glVertex2f(centerX, centerY + CROSSHAIR_LINE_END);

		_openglFunctions->glEnd();

		_openglFunctions->glPointSize(1);
		_openglFunctions->glLineWidth(1);

		_openglFunctions->glPopMatrix();
	}

	if (_entityContext->Settings->ShowGuidelines)
	{
		_openglFunctions->glMatrixMode(GL_PROJECTION);
		_openglFunctions->glLoadIdentity();

		_openglFunctions->glOrtho(0.0f, (float)_windowWidth, (float)_windowHeight, 0.0f, 1.0f, -1.0f);

		_openglFunctions->glMatrixMode(GL_MODELVIEW);
		_openglFunctions->glPushMatrix();
		_openglFunctions->glLoadIdentity();

		_openglFunctions->glDisable(GL_CULL_FACE);

		_openglFunctions->glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		_openglFunctions->glDisable(GL_TEXTURE_2D);

		_openglFunctions->glColor4fv(glm::value_ptr(glm::vec4{CrosshairColor, 1}));

		_openglFunctions->glPointSize(GUIDELINES_LINE_WIDTH);
		_openglFunctions->glLineWidth(GUIDELINES_LINE_WIDTH);

		_openglFunctions->glBegin(GL_POINTS);

		for (int yPos = _windowHeight - GUIDELINES_LINE_LENGTH; yPos >= centerY + CROSSHAIR_LINE_END; yPos -= GUIDELINES_OFFSET)
		{
			_openglFunctions->glVertex2f(centerX - GUIDELINES_LINE_WIDTH, yPos);
		}

		_openglFunctions->glEnd();

		_openglFunctions->glBegin(GL_LINES);

		for (int yPos = _windowHeight - GUIDELINES_LINE_LENGTH - GUIDELINES_POINT_LINE_OFFSET - GUIDELINES_LINE_WIDTH;
			yPos >= centerY + CROSSHAIR_LINE_END + GUIDELINES_LINE_LENGTH;
			yPos -= GUIDELINES_OFFSET)
		{
			_openglFunctions->glVertex2f(centerX, yPos);
			_openglFunctions->glVertex2f(centerX, yPos - GUIDELINES_LINE_LENGTH);
		}

		_openglFunctions->glEnd();

		const float flWidth = _windowHeight * (16 / 9.0);

		_openglFunctions->glLineWidth(GUIDELINES_EDGE_WIDTH);

		_openglFunctions->glBegin(GL_LINES);

		_openglFunctions->glVertex2f((_windowWidth / 2.) - (flWidth / 2), 0);
		_openglFunctions->glVertex2f((_windowWidth / 2.) - (flWidth / 2), _windowHeight);

		_openglFunctions->glVertex2f((_windowWidth / 2.) + (flWidth / 2), 0);
		_openglFunctions->glVertex2f((_windowWidth / 2.) + (flWidth / 2), _windowHeight);

		_openglFunctions->glEnd();

		_openglFunctions->glPointSize(1);
		_openglFunctions->glLineWidth(1);

		_openglFunctions->glPopMatrix();
	}
}

void Scene::DrawModel()
{
	auto camera = GetCurrentCamera();

	//
	// draw background
	//

	if (_entityContext->Settings->ShowBackground && BackgroundTexture != GL_INVALID_TEXTURE_ID)
	{
		graphics::DrawBackground(_openglFunctions, BackgroundTexture);
	}

	CollectRenderables(RenderPass::Background, _renderablesToRender);

	for (const auto& renderable : _renderablesToRender)
	{
		renderable->Draw(_openglFunctions, RenderPass::Background);
	}

	_openglFunctions->glMatrixMode(GL_PROJECTION);
	_openglFunctions->glLoadIdentity();
	_openglFunctions->glLoadMatrixf(glm::value_ptr(camera->GetProjectionMatrix()));

	_openglFunctions->glMatrixMode(GL_MODELVIEW);
	_openglFunctions->glPushMatrix();
	_openglFunctions->glLoadIdentity();
	_openglFunctions->glLoadMatrixf(glm::value_ptr(camera->GetViewMatrix()));

	if (_entityContext->Settings->ShowAxes)
	{
		_openglFunctions->glDisable(GL_TEXTURE_2D);
		_openglFunctions->glEnable(GL_DEPTH_TEST);

		const float flLength = 50.0f;

		_openglFunctions->glLineWidth(1.0f);

		_openglFunctions->glBegin(GL_LINES);

		_openglFunctions->glColor3f(1.0f, 0, 0);

		_openglFunctions->glVertex3f(0, 0, 0);
		_openglFunctions->glVertex3f(flLength, 0, 0);

		_openglFunctions->glColor3f(0, 1, 0);

		_openglFunctions->glVertex3f(0, 0, 0);
		_openglFunctions->glVertex3f(0, flLength, 0);

		_openglFunctions->glColor3f(0, 0, 1.0f);

		_openglFunctions->glVertex3f(0, 0, 0);
		_openglFunctions->glVertex3f(0, 0, flLength);

		_openglFunctions->glEnd();
	}

	_studioModelRenderer->SetOpenGLFunctions(_openglFunctions);

	_studioModelRenderer->SetViewerOrigin(camera->GetOrigin());
	_studioModelRenderer->SetViewerRight(camera->GetRightVector());

	const unsigned int uiOldPolys = _studioModelRenderer->GetDrawnPolygonsCount();

	//TODO: this is currently here so model rendering works. Eventually when everything is moved into renderables this needs to move.
	CollectRenderables(RenderPass::Standard, _renderablesToRender);

	for (const auto& renderable : _renderablesToRender)
	{
		renderable->Draw(_openglFunctions, RenderPass::Standard);
	}

	CollectRenderables(RenderPass::Overlay3D, _renderablesToRender);

	for (const auto& renderable : _renderablesToRender)
	{
		renderable->Draw(_openglFunctions, RenderPass::Overlay3D);
	}

	//
	// draw ground
	//

	if (_entityContext->Settings->ShowGround)
	{
		glm::vec2 textureOffset{0};

		//Calculate texture offset based on sequence movement and current frame
		if (_entity)
		{
			auto model = _entity->GetEditableModel();

			const int sequenceIndex = _entity->GetSequence();

			if (sequenceIndex >= 0 && sequenceIndex < model->Sequences.size())
			{
				const auto& sequence = *model->Sequences[_entity->GetSequence()];

				//Scale offset to current frame
				const float currentFrame = _entity->GetFrame() / (sequence.NumFrames - 1);

				float delta;

				if (currentFrame >= _previousFloorFrame)
				{
					delta = currentFrame - _previousFloorFrame;
				}
				else
				{
					delta = (currentFrame + 1) - _previousFloorFrame;
				}

				_previousFloorFrame = currentFrame;

				//Adjust scrolling direction based on whether the model is mirrored, but don't apply scale itself
				const int xDirection = _entity->GetScale().x > 0 ? 1 : -1;
				const int yDirection = _entity->GetScale().y > 0 ? 1 : -1;

				textureOffset.x = sequence.LinearMovement.x * delta * xDirection;
				textureOffset.y = -(sequence.LinearMovement.y * delta * yDirection);

				if (_floorSequence != _entity->GetSequence())
				{
					_floorSequence = _entity->GetSequence();
					_previousFloorFrame = 0;
					_floorTextureOffset.x = _floorTextureOffset.y = 0;
				}
			}
		}

		_floorTextureOffset += textureOffset;

		const float floorTextureLength = _entityContext->Settings->EnableFloorTextureTiling ? _entityContext->Settings->FloorTextureLength : _entityContext->Settings->GetFloorLength();

		//Prevent the offset from overflowing
		_floorTextureOffset.x = std::fmod(_floorTextureOffset.x, floorTextureLength);
		_floorTextureOffset.y = std::fmod(_floorTextureOffset.y, floorTextureLength);

		graphics::DrawFloor(_openglFunctions, _entityContext->Settings->FloorOrigin, _entityContext->Settings->GetFloorLength(), floorTextureLength, _floorTextureOffset, GroundTexture, GroundColor, _entityContext->Settings->MirrorOnGround);
	}

	_drawnPolygonsCount = _studioModelRenderer->GetDrawnPolygonsCount() - uiOldPolys;

	if (_entityContext->Settings->ShowPlayerHitbox)
	{
		//Draw a transparent green box to display the player hitbox
		const glm::vec3 bbmin{-16, -16, 0};
		const glm::vec3 bbmax{16, 16, 72};

		auto v = CreateBoxFromBounds(bbmin, bbmax);

		DrawOutlinedBox(_openglFunctions, v, {0.0f, 1.0f, 0.0f, 0.5f}, {0.0f, 0.5f, 0.0f, 1.f});
	}

	if (_entityContext->Settings->ShowBBox)
	{
		if (_entity)
		{
			//Draw a transparent brownish box to display the bounding box
			auto model = _entity->GetEditableModel();

			const auto v = CreateBoxFromBounds(model->BoundingMin, model->BoundingMax);

			DrawOutlinedBox(_openglFunctions, v, {1.0f, 1.0f, 0.0f, 0.5f}, {0.5f, 0.5f, 0.0f, 1.0f});
		}
	}

	if (_entityContext->Settings->ShowCBox)
	{
		if (_entity)
		{
			//Draw a transparent orangeish box to display the clipping box
			auto model = _entity->GetEditableModel();

			const auto v = CreateBoxFromBounds(model->ClippingMin, model->ClippingMax);

			DrawOutlinedBox(_openglFunctions, v, {1.0f, 0.5f, 0.0f, 0.5f}, {0.5f, 0.25f, 0.0f, 1.0f});
		}
	}

	_openglFunctions->glPopMatrix();
}

void Scene::CollectRenderables(RenderPass::RenderPass renderPass, std::vector<BaseEntity*>& renderablesToRender)
{
	renderablesToRender.clear();

	for (auto entity = _entityList->GetFirstEntity(); entity.IsValid(*_entityList); entity = _entityList->GetNextEntity(entity))
	{
		auto ent = entity.Get(*_entityList);

		if (ent->GetRenderPasses() & renderPass)
		{
			renderablesToRender.emplace_back(ent);
		}
	}
}
}
