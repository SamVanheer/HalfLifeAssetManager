#include <glm/vec3.hpp>

#include "shared/Const.h"

#include "StudioSorting.h"

namespace studiomdl
{
bool CompareSortedMeshes( const SortedMesh_t& lhs, const SortedMesh_t& rhs )
{
	if( ( lhs.flags & ( STUDIO_NF_ADDITIVE ) ) == 0 && rhs.flags & ( STUDIO_NF_ADDITIVE ) )
		return true;

	if( lhs.flags & ( STUDIO_NF_MASKED ) && ( rhs.flags & ( STUDIO_NF_MASKED ) ) == 0 )
		return true;

	return false;
}
}