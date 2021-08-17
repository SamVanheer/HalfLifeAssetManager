#include "entity/BaseEntity.hpp"
#include "entity/EHandle.hpp"
#include "entity/EntityList.hpp"

BaseEntity* EHandle::Get(const EntityList& entityList) const
{
	return entityList.GetEntityByHandle(*this);
}

void EHandle::Set(BaseEntity* entity)
{
	*this = entity ? entity->GetEntHandle() : EHandle{};
}
