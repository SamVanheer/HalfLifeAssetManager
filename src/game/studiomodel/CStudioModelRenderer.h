#ifndef GAME_STUDIOMODEL_CSTUDIOMODELRENDERER_H
#define GAME_STUDIOMODEL_CSTUDIOMODELRENDERER_H

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <glm/mat3x4.hpp>

#include "studio.h"

#include "utility/Color.h"

class CStudioModelEntity;

namespace studiomodel
{
class CStudioModel;

/**
*	Used to render studio models. Only one instance of this class should be used, and should be kept around, in order to achieve reasonably performant and consistent rendering.
*/
class CStudioModelRenderer final
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

	/**
	*	Initializes the renderer. This sets up the renderer's state for a single "map".
	*	@return true on success, false otherwise.
	*/
	bool Initialize();

	/**
	*	Shuts down the renderer. This clears all state.
	*/
	void Shutdown();

	/**
	*	Should be called once per game frame.
	*/
	void RunFrame();

	/**
	*	Gets the number of models that have been drawn during this map.
	*/
	unsigned int GetModelsDrawnCount() const { return m_uiModelsDrawnCount; }

	/**
	*	Gets the number of polygons drawn since the last call to Prepare.
	*/
	unsigned int GetDrawnPolygonsCount() const { return m_uiDrawnPolygonsCount; }

	/**
	*	Draws the given model.
	*/
	unsigned int DrawModel( CStudioModelEntity* const pEntity, const bool wireframeOnly = false );

private:
	void SetUpBones();
	void CalcRotations( glm::vec3 *pos, glm::vec4 *q, mstudioseqdesc_t *pseqdesc, mstudioanim_t *panim, float f );

	void CalcBoneAdj();
	void CalcBoneQuaternion( int frame, float s, mstudiobone_t *pbone, mstudioanim_t *panim, glm::vec4& q );
	void CalcBonePosition( int frame, float s, mstudiobone_t *pbone, mstudioanim_t *panim, glm::vec3& pos );
	void SlerpBones( glm::vec4* q1, glm::vec3* pos1, glm::vec4* q2, glm::vec3* pos2, float s );

	void SetupLighting();

	void SetupModel( int bodypart );

	unsigned int DrawPoints( const bool wireframeOnly = false );

	void Lighting( float *lv, int bone, int flags, const glm::vec3& normal );
	void Chrome( glm::ivec2& chrome, int bone, const glm::vec3& normal );

private:
	/**
	*	Total number of models drawn by this renderer since the last time it was initialized.
	*/
	unsigned int m_uiModelsDrawnCount = 0;

	/**
	*	The current model being rendered.
	*/
	CStudioModel* m_pCurrentModel = nullptr;
	CStudioModelEntity* m_pEntity = nullptr;

	studiohdr_t* m_pStudioHdr = nullptr;
	studiohdr_t* m_pTextureHdr = nullptr;

	mstudiomodel_t* m_pModel = nullptr;

	/**
	*	The number of polygons drawn since the last call to Prepare.
	*/
	unsigned int m_uiDrawnPolygonsCount = 0;

	glm::vec3		g_xformverts[ MAXSTUDIOVERTS ];		// transformed vertices
	glm::vec3		g_lightvalues[ MAXSTUDIOVERTS ];	// light surface normals
	glm::vec3*		g_pxformverts;
	glm::vec3*		g_pvlightvalues;

	glm::mat3x4		g_bonetransform[ MAXSTUDIOBONES ];	// bone transformation matrix

	glm::vec4		m_Adj;

	int				g_ambientlight;						// ambient world light
	float			g_shadelight;						// direct world light

	glm::vec3		g_lightvec;							// light vector in model reference frame
	Color			g_lightcolor;
	glm::vec3		g_blightvec[ MAXSTUDIOBONES ];		// light vectors in bone reference frames

	glm::ivec2		g_chrome[ MAXSTUDIOVERTS ];			// texture coords for surface normals
	unsigned int	g_chromeage[ MAXSTUDIOBONES ];		// last time chrome vectors were updated
	glm::vec3		g_chromeup[ MAXSTUDIOBONES ];		// chrome vector "up" in bone reference frames
	glm::vec3		g_chromeright[ MAXSTUDIOBONES ];	// chrome vector "right" in bone reference frames

private:
	CStudioModelRenderer( const CStudioModelRenderer& ) = delete;
	CStudioModelRenderer& operator=( const CStudioModelRenderer& ) = delete;
};

//TODO: avoid using a global
CStudioModelRenderer& renderer();
}

//TODO: move these into CStudioModelRenderer
extern glm::vec3 g_vright;		// needs to be set to viewer's right in order for chrome to work
extern float g_lambert;		// modifier for pseudo-hemispherical lighting

#endif //GAME_STUDIOMODEL_CSTUDIOMODELRENDERER_H