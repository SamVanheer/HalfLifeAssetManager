#ifndef GAME_STUDIOMODEL_STUDIOSORTING_H
#define GAME_STUDIOMODEL_STUDIOSORTING_H

#include "shared/studiomodel/studio.h"

namespace studiomdl
{
struct SortedMesh_t
{
	mstudiomesh_t* pMesh;
	int flags;
};

bool CompareSortedMeshes( const SortedMesh_t& lhs, const SortedMesh_t& rhs );
}

#endif //GAME_STUDIOMODEL_STUDIOSORTING_H
