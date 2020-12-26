#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <glm/mat3x4.hpp>

#include "engine/renderer/studiomodel/StudioSorting.hpp"
#include "engine/shared/renderer/studiomodel/IStudioModelRenderer.hpp"
#include "engine/shared/studiomodel/StudioModelFileFormat.hpp"

namespace studiomdl
{
class StudioModelRenderer final : public studiomdl::IStudioModelRenderer
{
public:
	StudioModelRenderer();
	~StudioModelRenderer();

	StudioModelRenderer(const StudioModelRenderer&) = delete;
	StudioModelRenderer& operator=(const StudioModelRenderer&) = delete;

	bool Initialize() override final;

	void Shutdown() override final;

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

	glm::vec3 GetLightColor() const override final { return _lightcolor; }

	void SetLightColor(const glm::vec3& color) override final
	{
		_lightcolor = color;
	}

	glm::vec3 GetWireframeColor() const override final { return _wireframeColor; }

	void SetWireframeColor(const glm::vec3& color) override final
	{
		_wireframeColor = color;
	}

	unsigned int DrawModel(ModelRenderInfo* const renderInfo, const renderer::DrawFlags flags) override final;

	void DrawSingleBone(ModelRenderInfo& renderInfo, const int iBone) override final;

	void DrawSingleAttachment(ModelRenderInfo& renderInfo, const int iAttachment) override final;

	void DrawSingleHitbox(ModelRenderInfo& renderInfo, const int hitboxIndex) override final;

private:
	void DrawBones();

	void DrawAttachments();

	void DrawEyePosition();

	void DrawHitBoxes();

	void DrawNormals();

	void SetUpBones();
	void CalcRotations(glm::vec3* pos, glm::vec4* q, const mstudioseqdesc_t* const pseqdesc, const mstudioanim_t* panim, const float f);

	void CalcBoneAdj();
	void CalcBoneQuaternion(const int frame, const float s, const mstudiobone_t* const pbone, const mstudioanim_t* const panim, glm::vec4& q);
	void CalcBonePosition(const int frame, const float s, const mstudiobone_t* const pbone, const mstudioanim_t* const panim, glm::vec3& pos);
	void SlerpBones(glm::vec4* q1, glm::vec3* pos1, glm::vec4* q2, glm::vec3* pos2, float s);

	/**
	*	@brief set some global variables based on entity position
	*/
	void SetupLighting();

	/**
	*	@brief based on the body part, figure out which mesh it should be using
	*/
	void SetupModel(int bodypart);

	unsigned int DrawPoints(const bool bWireframe);

	unsigned int DrawMeshes(const bool bWireframe, const SortedMesh* pMeshes, const mstudiotexture_t* pTextures, const short* pSkinRef);

	unsigned int DrawShadows(const bool fixZFighting, const bool wireframe);

	unsigned int InternalDrawShadows();

	void Lighting(glm::vec3& lv, int bone, int flags, const glm::vec3& normal);
	void Chrome(glm::vec2& chrome, int bone, const glm::vec3& normal);

private:
	/**
	*	Total number of models drawn by this renderer since the last time it was initialized.
	*/
	unsigned int _modelsDrawnCount = 0;

	studiomdl::ModelRenderInfo* _renderInfo;

	studiohdr_t* _studioHeader = nullptr;
	studiohdr_t* _textureHeader = nullptr;

	mstudiomodel_t* _model = nullptr;

	/**
	*	The number of polygons drawn since the last call to Initialize.
	*/
	unsigned int _drawnPolygonsCount = 0;

	glm::vec3		_xformverts[MAXSTUDIOVERTS];		// transformed vertices
	glm::vec3		_xformnorms[MAXSTUDIOVERTS];
	glm::vec3		_lightvalues[MAXSTUDIOVERTS];	// light surface normals

	glm::mat3x4		_bonetransform[MAXSTUDIOBONES];	// bone transformation matrix

	float			_adj[MAXSTUDIOCONTROLLERS];		//This used to be a vec4, but it really needs to be this.

	int				_ambientlight;						// ambient world light
	float			_shadelight;						// direct world light

	glm::vec3		_lightvec = {0, 0, -1};			// light vector in model reference frame
	glm::vec3		_lightcolor{255, 255, 255};
	glm::vec3		_blightvec[MAXSTUDIOBONES];		// light vectors in bone reference frames

	glm::vec2		_chrome[MAXSTUDIOVERTS];			// texture coords for surface normals
	unsigned int	_chromeage[MAXSTUDIOBONES];		// last time chrome vectors were updated
	glm::vec3		_chromeup[MAXSTUDIOBONES];		// chrome vector "up" in bone reference frames
	glm::vec3		_chromeright[MAXSTUDIOBONES];	// chrome vector "right" in bone reference frames

	glm::vec3		_viewerOrigin;
	glm::vec3		_viewerRight = {50, 50, 0};	// needs to be set to viewer's right in order for chrome to work
	float			_lambert = 1.5f;					// modifier for pseudo-hemispherical lighting

	glm::vec3 _wireframeColor{255, 0, 0};
};
}
