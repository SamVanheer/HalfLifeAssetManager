#pragma once

#include <glm/vec3.hpp>

#include "core/shared/Const.hpp"

#include "engine/shared/renderer/DrawConstants.hpp"

#include "CModelRenderInfo.hpp"

/**
*	@defgroup StudioModelRenderer StudioModel Renderer.
*
*	@{
*/

namespace studiomdl
{
class CStudioModel;
class IStudioModelRendererListener;

/**
*	Used to render studio models. Only one instance of this class should be used, and should be kept around, in order to achieve reasonably performant and consistent rendering.
*/
class IStudioModelRenderer
{
public:
	virtual ~IStudioModelRenderer() = 0 {}

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

	virtual glm::vec3 GetLightColor() const = 0;

	virtual void SetLightColor(const glm::vec3& color) = 0;

	virtual glm::vec3 GetWireframeColor() const = 0;

	virtual void SetWireframeColor(const glm::vec3& color) = 0;

	/**
	*	Draws the given model.
	*	@param pRenderInfo Render info that describes the model.
	*	@param flags Flags.
	*	@return Number of polygons that were drawn.
	*/
	virtual unsigned int DrawModel( CModelRenderInfo* const pRenderInfo, const renderer::DrawFlags_t flags = renderer::DrawFlag::NONE ) = 0;

	/*
	*	Tool only operations.
	*/

	/**
	*	@return The current renderer listener, if any.
	*/
	virtual IStudioModelRendererListener* GetRendererListener() const = 0;

	/**
	*	Sets the current renderer listener.
	*	@param pListener Listener to set. May be null.
	*/
	virtual void SetRendererListener( IStudioModelRendererListener* pListener ) = 0;

	/**
	*	Draws a single bone.
	*	@param iBone Index of the bone to draw.
	*/
	virtual void DrawSingleBone(CModelRenderInfo& renderInfo, const int iBone) = 0;

	/**
	*	Draws a single attachment.
	*	@param iAttachment Index of the attachment to draw.
	*/
	virtual void DrawSingleAttachment(CModelRenderInfo& renderInfo, const int iAttachment) = 0;

	virtual void DrawSingleHitbox(CModelRenderInfo& renderInfo, const int hitboxIndex) = 0;
};
}

/** @ } */
