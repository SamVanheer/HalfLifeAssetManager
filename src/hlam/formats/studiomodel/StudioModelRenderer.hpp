#pragma once

#include <memory>
#include <vector>

#include <spdlog/logger.h>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <glm/mat4x4.hpp>

#include "formats/DrawConstants.hpp"
#include "formats/studiomodel/BoneTransformer.hpp"
#include "formats/studiomodel/ModelRenderInfo.hpp"
#include "formats/studiomodel/StudioModelFileFormat.hpp"
#include "formats/studiomodel/StudioSorting.hpp"

#include "graphics/Light.hpp"

class QOpenGLFunctions_1_1;

class ColorSettings;

namespace studiomdl
{
struct StudioAnimation;
struct StudioBone;
struct StudioSubModel;
struct StudioSequence;

/**
*	Used to render studio models. Only one instance of this class should be used, and should be kept around,
*	in order to achieve reasonably performant and consistent rendering.
*/
class StudioModelRenderer final
{
public:
	StudioModelRenderer(const std::shared_ptr<spdlog::logger>& logger, QOpenGLFunctions_1_1* openglFunctions, ColorSettings* colorSettings);
	~StudioModelRenderer();

	StudioModelRenderer(const StudioModelRenderer&) = delete;
	StudioModelRenderer& operator=(const StudioModelRenderer&) = delete;

	void RunFrame();

	/**
	*	@return The number of polygons drawn since the last call to Initialize.
	*/
	unsigned int GetDrawnPolygonsCount() const { return _drawnPolygonsCount; }

	/**
	*	@return The current lambert value. Modifier for pseudo-hemispherical lighting.
	*/
	float GetLambert() const { return _lambert; }

	/**
	*	@return The viewer's origin.
	*	TODO: move to separate data structure.
	*/
	const glm::vec3& GetViewerOrigin() const { return _viewerOrigin; }

	void SetViewerOrigin(const glm::vec3& viewerOrigin)
	{
		_viewerOrigin = viewerOrigin;
	}

	/**
	*	@return The vector that points to the viewer's right.
	*	TODO: move to separate data structure.
	*/
	const glm::vec3& GetViewerRight() const { return _viewerRight; }

	void SetViewerRight(const glm::vec3& viewerRight)
	{
		_viewerRight = viewerRight;
	}

	const graphics::Light& GetSkyLight() const { return _skyLight; }

	void SetSkyLight(const graphics::Light& light)
	{
		_skyLight = light;
	}

	/**
	*	Draws the given model.
	*	@param renderInfo Render info that describes the model.
	*	@param flags Flags.
	*	@return Number of polygons that were drawn.
	*/
	unsigned int DrawModel(ModelRenderInfo& renderInfo, float floorHeight, const renderer::DrawFlags flags);

	/*
	*	Tool only operations.
	*/

	/**
	*	Draws a single bone.
	*	@param iBone Index of the bone to draw.
	*/
	void DrawSingleBone(ModelRenderInfo& renderInfo, const int iBone);

	/**
	*	Draws a single attachment.
	*	@param iAttachment Index of the attachment to draw.
	*/
	void DrawSingleAttachment(ModelRenderInfo& renderInfo, const int iAttachment);

	void DrawSingleHitbox(ModelRenderInfo& renderInfo, const int hitboxIndex);

private:
	void UpdateColors();

	void SetupPosition(const glm::vec3& origin, const glm::vec3& angles);

	void DrawBones();

	void DrawAttachments();

	void DrawEyePosition();

	void DrawHitBoxes();

	void DrawNormals();

	void SetUpBones();

	/**
	*	@brief set some global variables based on entity position
	*/
	void SetupLighting();

	/**
	*	@brief based on the body part, figure out which mesh it should be using
	*/
	void SetupModel(int bodypart);

	unsigned int DrawPoints(const bool bWireframe);

	unsigned int DrawMeshes(const bool bWireframe, const SortedMesh* pMeshes);

	unsigned int DrawShadows(const bool fixZFighting, const bool wireframe, float floorHeight);

	unsigned int InternalDrawShadows(float floorHeight);

	void Lighting(glm::vec3& lv, int bone, int flags, const glm::vec3& normal);
	void Chrome(glm::vec2& chrome, int bone, const glm::vec3& normal);

private:
	//TODO: need to validate model on load to ensure it does not exceed this limit
	static constexpr int MaxVertices = 0xFFFF;

	std::shared_ptr<spdlog::logger> _logger;

	QOpenGLFunctions_1_1* const _openglFunctions;

	ColorSettings* const _colorSettings;

	/**
	*	Total number of models drawn by this renderer since the last time it was initialized.
	*/
	unsigned int _modelsDrawnCount = 0;

	studiomdl::ModelRenderInfo* _renderInfo;

	studiomdl::EditableStudioModel* _studioModel{};

	const StudioSubModel* _model = nullptr;

	/**
	*	The number of polygons drawn since the last call to Initialize.
	*/
	unsigned int _drawnPolygonsCount = 0;

	glm::vec3		_xformverts[MaxVertices];		// transformed vertices
	glm::vec3		_xformnorms[MaxVertices];
	glm::vec3		_lightvalues[MaxVertices];	// light surface normals

	BoneTransformer _boneTransformer;

	const glm::mat4x4* _bonetransform{};	// bone transformation matrix

	graphics::Light _skyLight;
	glm::vec3		_blightvec[MAXSTUDIOBONES];		// light vectors in bone reference frames

	glm::vec2		_chrome[MaxVertices];			// texture coords for surface normals
	unsigned int	_chromeage[MAXSTUDIOBONES];		// last time chrome vectors were updated
	glm::vec3		_chromeup[MAXSTUDIOBONES];		// chrome vector "up" in bone reference frames
	glm::vec3		_chromeright[MAXSTUDIOBONES];	// chrome vector "right" in bone reference frames

	glm::vec3		_viewerOrigin;
	glm::vec3		_viewerRight = {50, 50, 0};	// needs to be set to viewer's right in order for chrome to work
	float			_lambert = 1.5f;					// modifier for pseudo-hemispherical lighting

	glm::vec3 _wireframeColor{1, 0, 0};
};
}
