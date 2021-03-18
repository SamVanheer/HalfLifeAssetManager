#pragma once

#include <cstdio>

namespace studiomdl
{
class EditableStudioModel;

void DumpModelInfo(FILE* file, const EditableStudioModel& model);
}
