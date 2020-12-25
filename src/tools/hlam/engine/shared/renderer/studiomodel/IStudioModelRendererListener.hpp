#pragma once

#include "engine/shared/renderer/studiomodel/CModelRenderInfo.hpp"

/**
*	@ingroup StudioModelRenderer
*
*	@{
*/

namespace studiomdl
{
class IStudioModelRenderer;

/**
*	@brief Listener used by the tool renderer to inform tools of draw stages.
*/
class IStudioModelRendererListener
{
public:
	virtual ~IStudioModelRendererListener() = 0;

	/**
	*	Called before the model is drawn. The renderer's state is ready at this point.
	*	@param renderer Renderer.
	*	@param info Render info.
	*/
	virtual void OnPreDraw( IStudioModelRenderer& renderer, const CModelRenderInfo& info ) {}

	/**
	*	Called after the model is drawn. The renderer's state is ready at this point.
	*	@param renderer Renderer.
	*	@param info Render info.
	*/
	virtual void OnPostDraw( IStudioModelRenderer& renderer, const CModelRenderInfo& info ) {}
};

inline IStudioModelRendererListener::~IStudioModelRendererListener()
{
}
}

/** @} */
