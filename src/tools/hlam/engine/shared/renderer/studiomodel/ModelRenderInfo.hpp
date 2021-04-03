#pragma once

#include <array>

#include <glm/vec3.hpp>

#include "core/shared/Const.hpp"

#include "engine/shared/studiomodel/StudioModelFileFormat.hpp"

/**
*	@ingroup StudioModelRenderer
*
*	@{
*/

namespace studiomdl
{
class EditableStudioModel;

/**
*	Data structure used to pass model render info into the engine.
*	TODO: this should only explicitly declare variables for studiomodel specific settings. Common settings should be accessed through a shared interface.
*/
struct ModelRenderInfo
{
	glm::vec3 Origin;
	glm::vec3 Angles;
	glm::vec3 Scale;

	EditableStudioModel* Model;

	float Transparency;

	int Sequence;
	float Frame;
	int Bodygroup;
	int Skin;

	std::array<byte, SequenceBlendCount> Blender;

	std::array<byte, ControllerCount> Controller;
	byte Mouth;
};
}

/** @} */
