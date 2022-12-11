#pragma once

namespace studiomdl
{
struct Mesh;

struct SortedMesh
{
	const Mesh* Mesh;
	int Flags;
};

bool CompareSortedMeshes(const SortedMesh& lhs, const SortedMesh& rhs);
}
