#ifndef ENGINE_STUDIOMODEL_ISTUDIOMODELRENDERER_H
#define ENGINE_STUDIOMODEL_ISTUDIOMODELRENDERER_H

#include <glm/vec3.hpp>

#include "lib/LibInterface.h"

#include "shared/Const.h"

#include "StudioModelConstants.h"

/**
*	@defgroup StudioModelRenderer StudioModel Renderer.
*
*	@{
*/

namespace studiomodel
{
class CStudioModel;
}

namespace studiomdl
{
/**
*	Data structure used to pass model render info into the engine.
*	TODO: this should only explicitly declare variables for studiomodel specific settings. Common settings should be accessed through a shared interface.
*/
struct CModelRenderInfo
{
	glm::vec3 vecOrigin;
	glm::vec3 vecAngles;
	glm::vec3 vecScale;

	studiomodel::CStudioModel* pModel;

	float flTransparency;

	int iSequence;
	float flFrame;
	int iBodygroup;
	int iSkin;

	byte iBlender[ 2 ];

	byte iController[ 4 ];
	byte iMouth;
};

/**
*	Used to render studio models. Only one instance of this class should be used, and should be kept around, in order to achieve reasonably performant and consistent rendering.
*/
class IStudioModelRenderer : public IBaseInterface
{
public:
	/**
	*	Initializes the renderer. This sets up the renderer's state for a single "map".
	*	@return true on success, false otherwise.
	*/
	virtual bool Initialize() = 0;

	/**
	*	Shuts down the renderer. This clears all state.
	*/
	virtual void Shutdown() = 0;

	/**
	*	Should be called once per game frame.
	*/
	virtual void RunFrame() = 0;

	/**
	*	@return The number of models that have been drawn during this map.
	*/
	virtual unsigned int GetModelsDrawnCount() const = 0;

	/**
	*	@return The number of polygons drawn since the last call to Initialize.
	*/
	virtual unsigned int GetDrawnPolygonsCount() const = 0;

	/**
	*	@return The current lambert value. Modifier for pseudo-hemispherical lighting.
	*/
	virtual float GetLambert() const = 0;

	/**
	*	@return The viewer's origin.
	*	TODO: move to separate data structure.
	*/
	virtual const glm::vec3& GetViewerOrigin() const = 0;

	/**
	*	Sets the viewer's origin.
	*	@param vecViewerOrigin Viewer origin.
	*/
	virtual void SetViewerOrigin( const glm::vec3& vecViewerOrigin ) = 0;

	/**
	*	@return The vector that points to the viewer's right.
	*	TODO: move to separate data structure.
	*/
	virtual const glm::vec3& GetViewerRight() const = 0;

	/**
	*	Sets the vector that points to the viewer's right.
	*	@param vecViewerRight Viewer right.
	*/
	virtual void SetViewerRight( const glm::vec3& vecViewerRight ) = 0;

	/**
	*	@return The current light vector.
	*/
	virtual const glm::vec3& GetLightVector() const = 0;

	/**
	*	Sets the current light vector.
	*	@param lightvec Light vector.
	*/
	virtual void SetLightVector( const glm::vec3& lightvec ) = 0;

	/**
	*	Draws the given model.
	*	@param pRenderInfo Render info that describes the model.
	*	@param flags Flags.
	*	@return Number of polygons that were drawn.
	*	TODO: remove dependency on entity itself.
	*/
	virtual unsigned int DrawModel( CModelRenderInfo* const pRenderInfo, const studiomodel::DrawFlags_t flags = studiomodel::DRAWF_NONE ) = 0;
};
}

/**
*	StudioModel Renderer interface name.
*/
#define ISTUDIOMODELRENDERER_NAME "IStudioModelRendererV001"

/** @ } */

#endif //ENGINE_STUDIOMODEL_ISTUDIOMODELRENDERER_H