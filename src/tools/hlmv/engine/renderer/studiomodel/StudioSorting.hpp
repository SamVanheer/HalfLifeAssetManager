#pragma once

#include "shared/studiomodel/studio.hpp"

namespace studiomdl
{
struct SortedMesh_t
{
	mstudiomesh_t* pMesh;
	int flags;
};

bool CompareSortedMeshes( const SortedMesh_t& lhs, const SortedMesh_t& rhs );
}
