#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <glm/mat3x4.hpp>

#include "utility/Color.hpp"

#include "shared/studiomodel/studio.hpp"

#include "shared/renderer/studiomodel/IStudioModelRenderer.hpp"

#include "StudioSorting.hpp"

namespace studiomdl
{
class CStudioModel;

class CStudioModelRenderer final : public studiomdl::IStudioModelRenderer
{
public:
	/**
	*	Constructor.
	*/
	CStudioModelRenderer();

	/**
	*	Destructor.
	*/
	~CStudioModelRenderer();

	bool Initialize() override final;

	void Shutdown() override final;

	void RunFrame() override final;

	unsigned int GetModelsDrawnCount() const override final { return m_uiModelsDrawnCount; }

	unsigned int GetDrawnPolygonsCount() const override final { return m_uiDrawnPolygonsCount; }

	float GetLambert() const override final { return m_flLambert; }

	const glm::vec3& GetViewerOrigin() const override final { return m_vecViewerOrigin; }

	void SetViewerOrigin( const glm::vec3& vecViewerOrigin ) override final
	{
		m_vecViewerOrigin = vecViewerOrigin;
	}

	const glm::vec3& GetViewerRight() const override final { return m_vecViewerRight; }

	void SetViewerRight( const glm::vec3& vecViewerRight ) override final { m_vecViewerRight = vecViewerRight; }

	const glm::vec3& GetLightVector() const override final { return m_lightvec; }

	void SetLightVector( const glm::vec3& lightvec ) override final
	{
		m_lightvec = lightvec;
	}

	Color GetLightColor() const override final { return m_lightcolor; }

	void SetLightColor(const Color& color) override final
	{
		m_lightcolor = color;
	}

	Color GetWireframeColor() const override final { return _wireframeColor; }

	void SetWireframeColor(const Color& color) override final
	{
		_wireframeColor = color;
	}

	unsigned int DrawModel( CModelRenderInfo* const pRenderInfo, const renderer::DrawFlags_t flags ) override final;

	IStudioModelRendererListener* GetRendererListener() const override final { return m_pListener; }

	void SetRendererListener( IStudioModelRendererListener* pListener ) override final
	{
		m_pListener = pListener;
	}

	void DrawSingleBone(CModelRenderInfo& renderInfo, const int iBone) override final;

	void DrawSingleAttachment(CModelRenderInfo& renderInfo, const int iAttachment) override final;

	void DrawSingleHitbox(CModelRenderInfo& renderInfo, const int hitboxIndex) override final;

private:
	void DrawBones();

	void DrawAttachments();

	void DrawEyePosition();

	void DrawHitBoxes();

	void DrawNormals();

	void SetUpBones();
	void CalcRotations( glm::vec3* pos, glm::vec4* q, const mstudioseqdesc_t* const pseqdesc, const mstudioanim_t* panim, const float f );

	void CalcBoneAdj();
	void CalcBoneQuaternion( const int frame, const float s, const mstudiobone_t* const pbone, const mstudioanim_t* const panim, glm::vec4& q );
	void CalcBonePosition( const int frame, const float s, const mstudiobone_t* const pbone, const mstudioanim_t* const panim, glm::vec3& pos );
	void SlerpBones( glm::vec4* q1, glm::vec3* pos1, glm::vec4* q2, glm::vec3* pos2, float s );

	/**
	*	@brief set some global variables based on entity position
	*/
	void SetupLighting();

	/**
	*	@brief based on the body part, figure out which mesh it should be using
	*/
	void SetupModel( int bodypart );

	unsigned int DrawPoints( const bool bWireframe );

	unsigned int DrawMeshes( const bool bWireframe, const SortedMesh_t* pMeshes, const mstudiotexture_t* pTextures, const short* pSkinRef );

	unsigned int DrawShadows(const bool fixZFighting, const bool wireframe);

	unsigned int InternalDrawShadows();

	void Lighting( glm::vec3& lv, int bone, int flags, const glm::vec3& normal );
	void Chrome( glm::vec2& chrome, int bone, const glm::vec3& normal );

private:
	/**
	*	Total number of models drawn by this renderer since the last time it was initialized.
	*/
	unsigned int m_uiModelsDrawnCount = 0;

	studiomdl::CModelRenderInfo* m_pRenderInfo;

	studiohdr_t* m_pStudioHdr = nullptr;
	studiohdr_t* m_pTextureHdr = nullptr;

	mstudiomodel_t* m_pModel = nullptr;

	IStudioModelRendererListener* m_pListener = nullptr;

	/**
	*	The number of polygons drawn since the last call to Initialize.
	*/
	unsigned int m_uiDrawnPolygonsCount = 0;

	glm::vec3		m_xformverts[ MAXSTUDIOVERTS ];		// transformed vertices
	glm::vec3		m_xformnorms[MAXSTUDIOVERTS];
	glm::vec3		m_lightvalues[ MAXSTUDIOVERTS ];	// light surface normals
	glm::vec3*		m_pxformverts;
	glm::vec3*		m_pvlightvalues;

	glm::mat3x4		m_bonetransform[ MAXSTUDIOBONES ];	// bone transformation matrix

	vec_t			m_Adj[ MAXSTUDIOCONTROLLERS ];		//This used to be a vec4, but it really needs to be this.

	int				m_ambientlight;						// ambient world light
	float			m_shadelight;						// direct world light

	glm::vec3		m_lightvec = { 0, 0, -1 };			// light vector in model reference frame
	Color			m_lightcolor{255, 255, 255};
	glm::vec3		m_blightvec[ MAXSTUDIOBONES ];		// light vectors in bone reference frames

	glm::vec2		m_chrome[ MAXSTUDIOVERTS ];			// texture coords for surface normals
	unsigned int	m_chromeage[ MAXSTUDIOBONES ];		// last time chrome vectors were updated
	glm::vec3		m_chromeup[ MAXSTUDIOBONES ];		// chrome vector "up" in bone reference frames
	glm::vec3		m_chromeright[ MAXSTUDIOBONES ];	// chrome vector "right" in bone reference frames

	glm::vec3		m_vecViewerOrigin;
	glm::vec3		m_vecViewerRight = { 50, 50, 0 };	// needs to be set to viewer's right in order for chrome to work
	float			m_flLambert = 1.5f;					// modifier for pseudo-hemispherical lighting

	Color _wireframeColor{255, 0, 0};

private:
	CStudioModelRenderer( const CStudioModelRenderer& ) = delete;
	CStudioModelRenderer& operator=( const CStudioModelRenderer& ) = delete;
};
}
