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

	if (MirrorOnGround)
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

	if (ShowCrosshair)
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

	if (ShowGuidelines)
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

	if (ShowBackground && BackgroundTexture != GL_INVALID_TEXTURE_ID)
	{
		graphics::DrawBackground(_openglFunctions, BackgroundTexture);
	}

	_openglFunctions->glMatrixMode(GL_PROJECTION);
	_openglFunctions->glLoadIdentity();
	_openglFunctions->glLoadMatrixf(glm::value_ptr(camera->GetProjectionMatrix()));

	_openglFunctions->glMatrixMode(GL_MODELVIEW);
	_openglFunctions->glPushMatrix();
	_openglFunctions->glLoadIdentity();
	_openglFunctions->glLoadMatrixf(glm::value_ptr(camera->GetViewMatrix()));

	if (ShowAxes)
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

	if (nullptr != _entity)
	{
		// setup stencil buffer and draw mirror
		if (MirrorOnGround)
		{
			graphics::DrawMirroredModel(_openglFunctions, 
				*_studioModelRenderer, _entity,
				CurrentRenderMode,
				ShowWireframeOverlay,
				FloorOrigin,
				FloorLength,
				EnableBackfaceCulling);
		}
	}

	graphics::SetupRenderMode(_openglFunctions, CurrentRenderMode, EnableBackfaceCulling);

	if (nullptr != _entity)
	{
		const glm::vec3& vecScale = _entity->GetScale();

		//Determine if an odd number of scale values are negative. The cull face has to be changed if so.
		const float flScale = vecScale.x * vecScale.y * vecScale.z;

		_openglFunctions->glCullFace(flScale > 0 ? GL_FRONT : GL_BACK);

		renderer::DrawFlags flags = renderer::DrawFlag::NONE;

		if (ShowWireframeOverlay)
		{
			flags |= renderer::DrawFlag::WIREFRAME_OVERLAY;
		}

		if (CameraIsFirstPerson)
		{
			flags |= renderer::DrawFlag::IS_VIEW_MODEL;
		}

		if (DrawShadows)
		{
			flags |= renderer::DrawFlag::DRAW_SHADOWS;
		}

		if (FixShadowZFighting)
		{
			flags |= renderer::DrawFlag::FIX_SHADOW_Z_FIGHTING;
		}

		//TODO: these should probably be made separate somehow
		if (ShowHitboxes)
		{
			flags |= renderer::DrawFlag::DRAW_HITBOXES;
		}

		if (ShowBones)
		{
			flags |= renderer::DrawFlag::DRAW_BONES;
		}

		if (ShowAttachments)
		{
			flags |= renderer::DrawFlag::DRAW_ATTACHMENTS;
		}

		if (ShowEyePosition)
		{
			flags |= renderer::DrawFlag::DRAW_EYE_POSITION;
		}

		if (ShowNormals)
		{
			flags |= renderer::DrawFlag::DRAW_NORMALS;
		}

		_entity->Draw(flags);

		auto renderInfo = _entity->GetRenderInfo();

		//TODO: this is a temporary hack. The graphics scene architecture needs a complete overhaul first,
		//then this can be done by rendering the model in a separate viewmodel layer
		if (CameraIsFirstPerson)
		{
			renderInfo.Origin.z -= 1;
		}

		if (DrawSingleBoneIndex != -1)
		{
			_entityContext->StudioModelRenderer->DrawSingleBone(renderInfo, DrawSingleBoneIndex);
		}

		if (DrawSingleAttachmentIndex != -1)
		{
			_entityContext->StudioModelRenderer->DrawSingleAttachment(renderInfo, DrawSingleAttachmentIndex);
		}

		if (DrawSingleHitboxIndex != -1)
		{
			_entityContext->StudioModelRenderer->DrawSingleHitbox(renderInfo, DrawSingleHitboxIndex);
		}
	}

	//
	// draw ground
	//

	if (ShowGround)
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

		const float floorTextureLength = EnableFloorTextureTiling ? FloorTextureLength : FloorLength;

		//Prevent the offset from overflowing
		_floorTextureOffset.x = std::fmod(_floorTextureOffset.x, floorTextureLength);
		_floorTextureOffset.y = std::fmod(_floorTextureOffset.y, floorTextureLength);

		graphics::DrawFloor(_openglFunctions, FloorOrigin, FloorLength, floorTextureLength, _floorTextureOffset, GroundTexture, GroundColor, MirrorOnGround);
	}

	_drawnPolygonsCount = _studioModelRenderer->GetDrawnPolygonsCount() - uiOldPolys;

	if (ShowPlayerHitbox)
	{
		//Draw a transparent green box to display the player hitbox
		const glm::vec3 bbmin{-16, -16, 0};
		const glm::vec3 bbmax{16, 16, 72};

		auto v = CreateBoxFromBounds(bbmin, bbmax);

		DrawOutlinedBox(_openglFunctions, v, {0.0f, 1.0f, 0.0f, 0.5f}, {0.0f, 0.5f, 0.0f, 1.f});
	}

	if (ShowBBox)
	{
		if (_entity)
		{
			//Draw a transparent brownish box to display the bounding box
			auto model = _entity->GetEditableModel();

			const auto v = CreateBoxFromBounds(model->BoundingMin, model->BoundingMax);

			DrawOutlinedBox(_openglFunctions, v, {1.0f, 1.0f, 0.0f, 0.5f}, {0.5f, 0.5f, 0.0f, 1.0f});
		}
	}

	if (ShowCBox)
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
}
