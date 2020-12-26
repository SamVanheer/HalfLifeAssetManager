#pragma once

#include "engine/shared/studiomodel/studio.hpp"

namespace studiomdl
{
struct SortedMesh
{
	mstudiomesh_t* Mesh;
	int Flags;
};

bool CompareSortedMeshes(const SortedMesh& lhs, const SortedMesh& rhs);
}
