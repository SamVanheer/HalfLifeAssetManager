#include <algorithm>
#include <cassert>

#include "entity/BaseEntity.hpp"

void BaseEntity::SetEntityContext(EntityContext* context)
{
	assert(context);
	assert(!_context);
	_context = context;
}

void BaseEntity::SetEntityList(EntityList* entityList)
{
	assert(entityList);
	assert(!_entityList);
	_entityList = entityList;
}

void BaseEntity::SetTransparency(const float transparency)
{
	_transparency = std::clamp(transparency, 0.f, 1.f);
}
