#include <glm/vec3.hpp>

#include "engine/renderer/studiomodel/StudioSorting.hpp"
#include "engine/shared/studiomodel/StudioModelFileFormat.hpp"

namespace studiomdl
{
bool CompareSortedMeshes(const SortedMesh& lhs, const SortedMesh& rhs)
{
	if ((lhs.Flags & (STUDIO_NF_ADDITIVE)) == 0 && rhs.Flags & (STUDIO_NF_ADDITIVE))
	{
		return true;
	}

	if (lhs.Flags & (STUDIO_NF_MASKED) && (rhs.Flags & (STUDIO_NF_MASKED)) == 0)
	{
		return true;
	}

	return false;
}
}
