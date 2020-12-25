#pragma once

#include <glm/vec3.hpp>

#include "core/shared/Const.hpp"

/**
*	@ingroup StudioModelRenderer
*
*	@{
*/

namespace studiomdl
{
class CStudioModel;

/**
*	Data structure used to pass model render info into the engine.
*	TODO: this should only explicitly declare variables for studiomodel specific settings. Common settings should be accessed through a shared interface.
*/
struct CModelRenderInfo
{
	glm::vec3 vecOrigin;
	glm::vec3 vecAngles;
	glm::vec3 vecScale;

	CStudioModel* pModel;

	float flTransparency;

	int iSequence;
	float flFrame;
	int iBodygroup;
	int iSkin;

	byte iBlender[ 2 ];

	byte iController[ 4 ];
	byte iMouth;
};
}

/** @} */
