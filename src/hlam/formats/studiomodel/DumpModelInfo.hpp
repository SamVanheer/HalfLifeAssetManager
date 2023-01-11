#pragma once

#include <cstdio>
#include <filesystem>

namespace studiomdl
{
class EditableStudioModel;

void DumpModelInfo(FILE* file, const std::filesystem::path& fileName, const EditableStudioModel& model);
}
