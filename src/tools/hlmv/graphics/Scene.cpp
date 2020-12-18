#include <GL/glew.h>

#include <glm/mat4x4.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "engine/renderer/studiomodel/CStudioModelRenderer.h"
#include "engine/shared/renderer/studiomodel/IStudioModelRenderer.h"
#include "entity/CHLMVStudioModelEntity.h"

#include "game/entity/CBaseEntityList.h"
#include "game/entity/CEntityManager.h"

#include "graphics/GraphicsHelpers.h"
#include "graphics/GraphicsUtils.h"
#include "graphics/IGraphicsContext.hpp"
#include "graphics/Scene.hpp"

#include "utility/Color.h"

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

Scene::Scene(soundsystem::ISoundSystem* soundSystem, CWorldTime* worldTime)
	: _studioModelRenderer(std::make_unique<studiomdl::CStudioModelRenderer>())
	, _worldTime(worldTime)
	//Use the default list class for now
	, _entityManager(std::make_unique<CEntityManager>(std::make_unique<CBaseEntityList>(), _worldTime))
	, _entityContext(std::make_unique<EntityContext>(_worldTime, _studioModelRenderer.get(), _entityManager->GetEntityList(), _entityManager.get(),
		soundSystem))
{
}

Scene::~Scene()
{
	if (BackgroundTexture != 0)
	{
		glDeleteTextures(1, &BackgroundTexture);
	}

	if (GroundTexture != 0)
	{
		glDeleteTextures(1, &GroundTexture);
	}
}

void Scene::SetGraphicsContext(std::unique_ptr<IGraphicsContext>&& graphicsContext)
{
	_graphicsContext = std::move(graphicsContext);
}

void Scene::SetEntity(CHLMVStudioModelEntity* entity)
{
	_entity = entity;

	glm::vec3 min, max;
	_entity->ExtractBbox(min, max);

	//Clamp the values to a reasonable range
	for (int i = 0; i < 3; ++i)
	{
		//Use different limits for min and max so centering won't end up setting origin to 0 0 0
		min[i] = clamp(min[i], -2000.f, 2000.f);
		max[i] = clamp(max[i], -1000.f, 1000.f);
	}

	float dx = max[0] - min[0];
	float dy = max[1] - min[1];
	float dz = max[2] - min[2];

	float d = dx;

	if (dy > d)
		d = dy;
	if (dz > d)
		d = dz;

	glm::vec3 trans;
	glm::vec3 rot;

	trans[2] = 0;
	trans[0] = -(min[2] + dz / 2);
	trans[1] = d * 1.0f;
	rot[0] = -90.0f;
	rot[1] = 0.0f;
	rot[2] = -90.0f;

	_camera.SetOrigin(trans);
	_camera.SetViewDirection(rot);
}

void Scene::AlignOnGround()
{
	auto entity = GetEntity();

	auto model = entity->GetModel();

	auto header = model->GetStudioHeader();

	//First try finding the idle sequence, since that typically represents a model "at rest"
	//Failing that, use the first sequence
	auto idleFinder = [&]() -> const mstudioseqdesc_t*
	{
		for (int i = 0; i < header->numseq; ++i)
		{
			const auto sequence = header->GetSequence(i);

			if (!strcmp(sequence->label, "idle"))
			{
				return sequence;
			}
		}

		return nullptr;
	};

	auto sequence = idleFinder();

	if (!sequence)
	{
		sequence = header->GetSequence(0);
	}

	entity->SetOrigin({0, 0, -sequence->bbmin.z});
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

	if (nullptr != _entity)
	{
		//TODO: should be replaced with an on-demand resource uploading stage in Draw()
		_entity->GetModel()->CreateTextures();
	}

	glGenTextures(1, &UVMeshTexture);
}

void Scene::Shutdown()
{
	--_initializeCount;

	if (_initializeCount != 0)
	{
		return;
	}

	glDeleteTexture(UVMeshTexture);
	UVMeshTexture = 0;

	_studioModelRenderer->Shutdown();
}

void Scene::Tick()
{
	_entityManager->RunFrame();
}

void Scene::Draw()
{
	glClearColor(_backgroundColor.r, _backgroundColor.g, _backgroundColor.b, 1.0f);

	if (MirrorOnGround)
	{
		glClearStencil(0);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	}
	else
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glViewport(0, 0, _windowWidth, _windowHeight);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	_drawnPolygonsCount = 0;

	if (ShowTexture)
	{
		DrawTexture(TextureXOffset, TextureYOffset, _windowWidth, _windowHeight, _entity, TextureIndex, TextureScale, ShowUVMap, OverlayUVMap);
	}
	else
	{
		DrawModel();
	}

	const int centerX = _windowWidth / 2;
	const int centerY = _windowHeight / 2;

	if (ShowCrosshair)
	{
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		glOrtho(0.0f, (float)_windowWidth, (float)_windowHeight, 0.0f, 1.0f, -1.0f);

		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();

		glDisable(GL_CULL_FACE);

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		glDisable(GL_TEXTURE_2D);

		//TODO:
		const Color crosshairColor = Color(255, 0, 0);//; m_pHLMV->GetSettings()->GetCrosshairColor();

		glColor4f(crosshairColor.GetRed() / 255.0f, crosshairColor.GetGreen() / 255.0f, crosshairColor.GetBlue() / 255.0f, 1.0);

		glPointSize(CROSSHAIR_LINE_WIDTH);
		glLineWidth(CROSSHAIR_LINE_WIDTH);

		glBegin(GL_POINTS);

		glVertex2f(centerX - CROSSHAIR_LINE_WIDTH / 2, centerY + 1);

		glEnd();

		glBegin(GL_LINES);

		glVertex2f(centerX - CROSSHAIR_LINE_START, centerY);
		glVertex2f(centerX - CROSSHAIR_LINE_END, centerY);

		glVertex2f(centerX + CROSSHAIR_LINE_START, centerY);
		glVertex2f(centerX + CROSSHAIR_LINE_END, centerY);

		glVertex2f(centerX, centerY - CROSSHAIR_LINE_START);
		glVertex2f(centerX, centerY - CROSSHAIR_LINE_END);

		glVertex2f(centerX, centerY + CROSSHAIR_LINE_START);
		glVertex2f(centerX, centerY + CROSSHAIR_LINE_END);

		glEnd();

		glPointSize(1);
		glLineWidth(1);

		glPopMatrix();
	}

	if (ShowGuidelines)
	{
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		glOrtho(0.0f, (float)_windowWidth, (float)_windowHeight, 0.0f, 1.0f, -1.0f);

		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();

		glDisable(GL_CULL_FACE);

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		glDisable(GL_TEXTURE_2D);

		//TODO: implement
		const Color crosshairColor = Color{255, 0, 0};// m_pHLMV->GetSettings()->GetCrosshairColor();

		glColor4f(crosshairColor.GetRed() / 255.0f, crosshairColor.GetGreen() / 255.0f, crosshairColor.GetBlue() / 255.0f, 1.0);

		glPointSize(GUIDELINES_LINE_WIDTH);
		glLineWidth(GUIDELINES_LINE_WIDTH);

		glBegin(GL_POINTS);

		for (int yPos = _windowHeight - GUIDELINES_LINE_LENGTH; yPos >= centerY + CROSSHAIR_LINE_END; yPos -= GUIDELINES_OFFSET)
		{
			glVertex2f(centerX - GUIDELINES_LINE_WIDTH, yPos);
		}

		glEnd();

		glBegin(GL_LINES);

		for (int yPos = _windowHeight - GUIDELINES_LINE_LENGTH - GUIDELINES_POINT_LINE_OFFSET - GUIDELINES_LINE_WIDTH;
			yPos >= centerY + CROSSHAIR_LINE_END + GUIDELINES_LINE_LENGTH;
			yPos -= GUIDELINES_OFFSET)
		{
			glVertex2f(centerX, yPos);
			glVertex2f(centerX, yPos - GUIDELINES_LINE_LENGTH);
		}

		glEnd();

		const float flWidth = _windowHeight * (16 / 9.0);

		glLineWidth(GUIDELINES_EDGE_WIDTH);

		glBegin(GL_LINES);

		glVertex2f((_windowWidth / 2) - (flWidth / 2), 0);
		glVertex2f((_windowWidth / 2) - (flWidth / 2), _windowHeight);

		glVertex2f((_windowWidth / 2) + (flWidth / 2), 0);
		glVertex2f((_windowWidth / 2) + (flWidth / 2), _windowHeight);

		glEnd();

		glPointSize(1);
		glLineWidth(1);

		glPopMatrix();
	}
}

void Scene::ApplyCameraToScene()
{
	//TODO: reimplement cameras
	auto pCamera = &_camera;// m_pHLMV->GetState()->GetCurrentCamera();

	const auto& vecOrigin = pCamera->GetOrigin();
	const auto vecAngles = pCamera->GetViewDirection();

	const glm::mat4x4 identity = Mat4x4ModelView();

	auto mat = Mat4x4ModelView();

	mat *= glm::translate(-vecOrigin);

	mat *= glm::rotate(glm::radians(vecAngles[2]), glm::vec3{1, 0, 0});

	mat *= glm::rotate(glm::radians(vecAngles[0]), glm::vec3{0, 1, 0});

	mat *= glm::rotate(glm::radians(vecAngles[1]), glm::vec3{0, 0, 1});

	glLoadMatrixf(glm::value_ptr(mat));
}

void Scene::SetupRenderMode(RenderMode renderMode)
{
	if (renderMode == RenderMode::INVALID)
		renderMode = CurrentRenderMode;

	graphics::helpers::SetupRenderMode(renderMode, EnableBackfaceCulling);
}

void Scene::DrawModel()
{
	//
	// draw background
	//

	if (ShowBackground && BackgroundTexture != GL_INVALID_TEXTURE_ID && !ShowTexture)
	{
		graphics::DrawBackground(BackgroundTexture);
	}

	graphics::SetProjection(*CurrentFOV, _windowWidth, _windowHeight);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	ApplyCameraToScene();

	if (ShowAxes)
	{
		glDisable(GL_TEXTURE_2D);
		glEnable(GL_DEPTH_TEST);

		const float flLength = 50.0f;

		glLineWidth(1.0f);

		glBegin(GL_LINES);

		glColor3f(1.0f, 0, 0);

		glVertex3f(0, 0, 0);
		glVertex3f(flLength, 0, 0);

		glColor3f(0, 1, 0);

		glVertex3f(0, 0, 0);
		glVertex3f(0, flLength, 0);

		glColor3f(0, 0, 1.0f);

		glVertex3f(0, 0, 0);
		glVertex3f(0, 0, flLength);

		glEnd();
	}

	//TODO: reimplement cameras
	auto camera = &_camera;//m_pHLMV->GetState()->GetCurrentCamera()

	const auto vecAngles = camera->GetViewDirection();

	auto mat = Mat4x4ModelView();

	mat *= glm::translate(-camera->GetOrigin());

	mat *= glm::rotate(glm::radians(vecAngles[2]), glm::vec3{1, 0, 0});

	mat *= glm::rotate(glm::radians(vecAngles[0]), glm::vec3{0, 1, 0});

	mat *= glm::rotate(glm::radians(vecAngles[1]), glm::vec3{0, 0, 1});

	const auto vecAbsOrigin = glm::inverse(mat)[3];

	_studioModelRenderer->SetViewerOrigin(glm::vec3(vecAbsOrigin));

	//Originally this was calculated as:
	//vecViewerRight[ 0 ] = vecViewerRight[ 1 ] = vecOrigin[ 2 ];
	//But that vector was incorrect. It mostly affects chrome because of its reflective nature.

	//Grab the angles that the player would have in-game. Since model viewer rotates the world, rather than moving the camera, this has to be adjusted.
	glm::vec3 angViewerDir = -camera->GetViewDirection();

	angViewerDir = angViewerDir + 180.0f;

	glm::vec3 vecViewerRight;

	//We're using the up vector here since the in-game look can only be matched if chrome is rotated.
	AngleVectors(angViewerDir, nullptr, nullptr, &vecViewerRight);

	//Invert it so it points down instead of up. This allows chrome to match the in-game look.
	_studioModelRenderer->SetViewerRight(-vecViewerRight);

	const unsigned int uiOldPolys = _studioModelRenderer->GetDrawnPolygonsCount();

	if (nullptr != _entity)
	{
		// setup stencil buffer and draw mirror
		if (MirrorOnGround)
		{
			graphics::helpers::DrawMirroredModel(*_studioModelRenderer, _entity,
				CurrentRenderMode,
				ShowWireframeOverlay,
				FloorLength,
				EnableBackfaceCulling);
		}
	}

	SetupRenderMode();

	if (nullptr != _entity)
	{
		const glm::vec3& vecScale = _entity->GetScale();

		//Determine if an odd number of scale values are negative. The cull face has to be changed if so.
		const float flScale = vecScale.x * vecScale.y * vecScale.z;

		glCullFace(flScale > 0 ? GL_FRONT : GL_BACK);

		renderer::DrawFlags_t flags = renderer::DrawFlag::NONE;

		if (ShowWireframeOverlay)
		{
			flags |= renderer::DrawFlag::WIREFRAME_OVERLAY;
		}

		//TODO: implement
#if false
		if (m_pHLMV->GetState()->UsingWeaponOrigin())
		{
			flags |= renderer::DrawFlag::IS_VIEW_MODEL;
		}
#endif

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
		//TODO: implement settings
		graphics::helpers::DrawFloor(FloorLength, GroundTexture,/* m_pHLMV->GetSettings()->GetGroundColor()*/{255, 0, 0}, MirrorOnGround);
	}

	_drawnPolygonsCount = _studioModelRenderer->GetDrawnPolygonsCount() - uiOldPolys;

	if (ShowPlayerHitbox)
	{
		//Draw a transparent green box to display the player hitbox
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glColor4f(0.0f, 1.0f, 0.0f, 0.5f);

		const glm::vec3 bbmin{-16, -16, 0};
		const glm::vec3 bbmax{16, 16, 72};

		glm::vec3 v[8];

		v[0][0] = bbmin[0];
		v[0][1] = bbmax[1];
		v[0][2] = bbmin[2];

		v[1][0] = bbmin[0];
		v[1][1] = bbmin[1];
		v[1][2] = bbmin[2];

		v[2][0] = bbmax[0];
		v[2][1] = bbmax[1];
		v[2][2] = bbmin[2];

		v[3][0] = bbmax[0];
		v[3][1] = bbmin[1];
		v[3][2] = bbmin[2];

		v[4][0] = bbmax[0];
		v[4][1] = bbmax[1];
		v[4][2] = bbmax[2];

		v[5][0] = bbmax[0];
		v[5][1] = bbmin[1];
		v[5][2] = bbmax[2];

		v[6][0] = bbmin[0];
		v[6][1] = bbmax[1];
		v[6][2] = bbmax[2];

		v[7][0] = bbmin[0];
		v[7][1] = bbmin[1];
		v[7][2] = bbmax[2];

		graphics::DrawBox(v);

		//Draw dark green edges
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glColor4f(0.0f, 0.5f, 0.0f, 0.5f);

		graphics::DrawBox(v);
	}

	glPopMatrix();
}

void Scene::DrawTexture(const int xOffset, const int yOffset, const int width, const int height, CStudioModelEntity* entity,
	const int textureIndex, const float textureScale, const bool showUVMap, const bool overlayUVMap)
{
	assert(entity);

	const auto model = entity->GetModel();

	assert(model);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glOrtho(0.0f, (float)width, (float)height, 0.0f, 1.0f, -1.0f);

	const studiohdr_t* const header = model->GetTextureHeader();

	assert(header);

	const mstudiotexture_t& texture = *header->GetTexture(textureIndex);

	const float w = texture.width * textureScale;
	const float h = texture.height * textureScale;

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glDisable(GL_CULL_FACE);
	glDisable(GL_BLEND);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	const float x = ((static_cast<float>(width) - w) / 2) + xOffset;
	const float y = ((static_cast<float>(height) - h) / 2) + yOffset;

	glDisable(GL_DEPTH_TEST);

	if (showUVMap && !overlayUVMap)
	{
		glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
		glDisable(GL_TEXTURE_2D);
		glRectf(x, y, x + w, y + h);
	}

	if (!showUVMap || overlayUVMap)
	{
		if (texture.flags & STUDIO_NF_MASKED)
		{
			glEnable(GL_ALPHA_TEST);
			glAlphaFunc(GL_GREATER, 0.5f);
		}

		glEnable(GL_TEXTURE_2D);
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		glBindTexture(GL_TEXTURE_2D, model->GetTextureId(textureIndex));

		glBegin(GL_TRIANGLE_STRIP);

		glTexCoord2f(0, 0);
		glVertex2f(x, y);

		glTexCoord2f(1, 0);
		glVertex2f(x + w, y);

		glTexCoord2f(0, 1);
		glVertex2f(x, y + h);

		glTexCoord2f(1, 1);
		glVertex2f(x + w, y + h);

		glEnd();

		glBindTexture(GL_TEXTURE_2D, 0);

		if (texture.flags & STUDIO_NF_MASKED)
		{
			glDisable(GL_ALPHA_TEST);
		}
	}

	if (showUVMap)
	{
		glEnable(GL_ALPHA_TEST);
		glAlphaFunc(GL_GREATER, 0.1f);

		glEnable(GL_TEXTURE_2D);
			
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		glBindTexture(GL_TEXTURE_2D, UVMeshTexture);

		glBegin(GL_TRIANGLE_STRIP);

		glTexCoord2f(0, 0);
		glVertex2f(x, y);

		glTexCoord2f(1, 0);
		glVertex2f(x + w, y);

		glTexCoord2f(0, 1);
		glVertex2f(x, y + h);

		glTexCoord2f(1, 1);
		glVertex2f(x + w, y + h);

		glEnd();

		glBindTexture(GL_TEXTURE_2D, 0);
		glDisable(GL_ALPHA_TEST);
	}

	glPopMatrix();

	glClear(GL_DEPTH_BUFFER_BIT);
}
}
