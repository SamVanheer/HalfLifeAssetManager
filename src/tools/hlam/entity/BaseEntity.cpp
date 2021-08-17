#include <algorithm>
#include <cassert>

#include "entity/BaseEntity.hpp"

void BaseEntity::SetEntityContext(EntityContext* context)
{
	assert(context);
	_context = context;
}

void BaseEntity::SetTransparency(const float transparency)
{
	_transparency = std::clamp(transparency, 0.f, 1.f);
}
