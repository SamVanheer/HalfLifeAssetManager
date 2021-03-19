#pragma once

#include <filesystem>

#include "engine/shared/studiomodel/EditableStudioModel.hpp"
#include "engine/shared/studiomodel/StudioModel.hpp"

namespace studiomdl
{
class StudioModel;

EditableStudioModel ConvertToEditable(const StudioModel& studioModel);
StudioModel ConvertFromEditable(const std::filesystem::path& fileName, const EditableStudioModel& studioModel);

/**
*	Returns the string representation for a studio control value.
*	@param iControl Value containing a STUDIO_* control bit.
*	@return String representation for the control, or nullptr.
*/
const char* ControlToString(const int iControl);

/**
*	Returns the description for a studio control value.
*	@param iControl Value containing a STUDIO_* control bit.
*	@return String description for the control, or nullptr.
*/
const char* ControlToStringDescription(const int iControl);

/**
*	Converts a string to a control value.
*	@param pszString String representation of a control value.
*	@return Control value, or -1 if the string does not represent a valid control.
*/
int StringToControl(const char* const pszString);
}
