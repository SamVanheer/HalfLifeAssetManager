#pragma once

#include <filesystem>

#include "engine/shared/studiomodel/EditableStudioModel.hpp"
#include "engine/shared/studiomodel/StudioModel.hpp"

namespace studiomdl
{
class StudioModel;

EditableStudioModel ConvertToEditable(const StudioModel& studioModel);
StudioModel ConvertFromEditable(const std::filesystem::path& fileName, const EditableStudioModel& studioModel);
}
