#pragma once

#include <glm/vec3.hpp>

#include "formats/DrawConstants.hpp"

#include "formats/studiomodel/ModelRenderInfo.hpp"

namespace graphics
{
struct Light;
}

/**
*	@defgroup StudioModelRenderer StudioModel Renderer.
*
*	@{
*/

namespace studiomdl
{
/**
*	Used to render studio models. Only one instance of this class should be used, and should be kept around, in order to achieve reasonably performant and consistent rendering.
*/
class IStudioModelRenderer
{
public:
	virtual ~IStudioModelRenderer() {}

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
	*/
	virtual void SetViewerOrigin(const glm::vec3& viewerOrigin) = 0;

	/**
	*	@return The vector that points to the viewer's right.
	*	TODO: move to separate data structure.
	*/
	virtual const glm::vec3& GetViewerRight() const = 0;

	/**
	*	Sets the vector that points to the viewer's right.
	*/
	virtual void SetViewerRight(const glm::vec3& viewerRight) = 0;

	virtual const graphics::Light& GetSkyLight() const = 0;

	virtual void SetSkyLight(const graphics::Light& light) = 0;

	/**
	*	Draws the given model.
	*	@param renderInfo Render info that describes the model.
	*	@param flags Flags.
	*	@return Number of polygons that were drawn.
	*/
	virtual unsigned int DrawModel(ModelRenderInfo& renderInfo, const renderer::DrawFlags flags = renderer::DrawFlag::NONE) = 0;

	/*
	*	Tool only operations.
	*/

	/**
	*	Draws a single bone.
	*	@param iBone Index of the bone to draw.
	*/
	virtual void DrawSingleBone(ModelRenderInfo& renderInfo, const int iBone) = 0;

	/**
	*	Draws a single attachment.
	*	@param iAttachment Index of the attachment to draw.
	*/
	virtual void DrawSingleAttachment(ModelRenderInfo& renderInfo, const int iAttachment) = 0;

	virtual void DrawSingleHitbox(ModelRenderInfo& renderInfo, const int hitboxIndex) = 0;
};
}

/** @ } */
