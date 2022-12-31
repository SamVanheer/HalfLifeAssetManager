#pragma once

namespace studiomdl
{
struct StudioMesh;

struct SortedMesh
{
	const StudioMesh* Mesh;
	int Flags;
};

bool CompareSortedMeshes(const SortedMesh& lhs, const SortedMesh& rhs);
}
