#include <cassert>

#include "game/entity/BaseEntity.hpp"

BaseEntity::BaseEntity() = default;
BaseEntity::~BaseEntity() = default;

void BaseEntity::Construct(const char* const className, EntityContext* context)
{
	assert(className && *className);
	assert(context);

	_className = className;
	_context = context;
}

void BaseEntity::OnCreate()
{
}

void BaseEntity::OnDestroy()
{
}

bool BaseEntity::Spawn()
{
	return true;
}
