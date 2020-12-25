#pragma once

#include <cstdio>

namespace studiomdl
{
class CStudioModel;

void DumpModelInfo(FILE* file, const CStudioModel& model);
}
