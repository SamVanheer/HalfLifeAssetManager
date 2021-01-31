#pragma once

#include <cstdio>

namespace studiomdl
{
class StudioModel;

void DumpModelInfo(FILE* file, const StudioModel& model);
}
