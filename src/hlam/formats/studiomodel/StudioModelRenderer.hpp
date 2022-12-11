#pragma once

#include <memory>
#include <vector>

#include <spdlog/logger.h>

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <glm/mat4x4.hpp>

#include "formats/studiomodel/BoneTransformer.hpp"
#include "formats/studiomodel/IStudioModelRenderer.hpp"
#include "formats/studiomodel/StudioModelFileFormat.hpp"
#include "formats/studiomodel/StudioSorting.hpp"

class QOpenGLFunctions_1_1;

namespace ui::settings
{
class ColorSettings;
}

namespace studiomdl
{
struct Animation;
struct Bone;
struct Model;
struct Sequence;

class StudioModelRenderer final : public studiomdl::IStudioModelRenderer
{
public:
	StudioModelRenderer(const std::shared_ptr<spdlog::logger>& logger, QOpenGLFunctions_1_1* openglFunctions, ui::settings::ColorSettings* colorSettings);
	~StudioModelRenderer();

	StudioModelRenderer(const StudioModelRenderer&) = delete;
	StudioModelRenderer& operator=(const StudioModelRenderer&) = delete;

	void RunFrame() override final;

	unsigned int GetModelsDrawnCount() const override final { return _modelsDrawnCount; }

	unsigned int GetDrawnPolygonsCount() const override final { return _drawnPolygonsCount; }

	float GetLambert() const override final { return _lambert; }

	const glm::vec3& GetViewerOrigin() const override final { return _viewerOrigin; }

	void SetViewerOrigin(const glm::vec3& viewerOrigin) override final
	{
		_viewerOrigin = viewerOrigin;
	}

	const glm::vec3& GetViewerRight() const override final { return _viewerRight; }

	void SetViewerRight(const glm::vec3& viewerRight) override final
	{
		_viewerRight = viewerRight;
	}

	const glm::vec3& GetLightVector() const override final { return _lightvec; }

	void SetLightVector(const glm::vec3& lightvec) override final
	{
		_lightvec = lightvec;
	}

	unsigned int DrawModel(ModelRenderInfo& renderInfo, const renderer::DrawFlags flags) override final;

	void DrawSingleBone(ModelRenderInfo& renderInfo, const int iBone) override final;

	void DrawSingleAttachment(ModelRenderInfo& renderInfo, const int iAttachment) override final;

	void DrawSingleHitbox(ModelRenderInfo& renderInfo, const int hitboxIndex) override final;

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

	unsigned int DrawShadows(const bool fixZFighting, const bool wireframe);

	unsigned int InternalDrawShadows();

	void Lighting(glm::vec3& lv, int bone, int flags, const glm::vec3& normal);
	void Chrome(glm::vec2& chrome, int bone, const glm::vec3& normal);

private:
	//TODO: need to validate model on load to ensure it does not exceed this limit
	static constexpr int MaxVertices = 0xFFFF;

	std::shared_ptr<spdlog::logger> _logger;

	QOpenGLFunctions_1_1* const _openglFunctions;

	ui::settings::ColorSettings* const _colorSettings;

	/**
	*	Total number of models drawn by this renderer since the last time it was initialized.
	*/
	unsigned int _modelsDrawnCount = 0;

	studiomdl::ModelRenderInfo* _renderInfo;

	studiomdl::EditableStudioModel* _studioModel{};

	const Model* _model = nullptr;

	/**
	*	The number of polygons drawn since the last call to Initialize.
	*/
	unsigned int _drawnPolygonsCount = 0;

	glm::vec3		_xformverts[MaxVertices];		// transformed vertices
	glm::vec3		_xformnorms[MaxVertices];
	glm::vec3		_lightvalues[MaxVertices];	// light surface normals

	BoneTransformer _boneTransformer;

	const glm::mat4x4* _bonetransform{};	// bone transformation matrix

	int				_ambientlight;						// ambient world light
	float			_shadelight;						// direct world light

	glm::vec3		_lightvec = {0, 0, -1};			// light vector in model reference frame
	glm::vec3		_lightcolor{1, 1, 1};
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
