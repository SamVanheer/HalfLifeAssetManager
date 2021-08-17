#include "game/entity/BaseEntity.hpp"
#include "game/entity/EHandle.hpp"
#include "game/entity/EntityList.hpp"

BaseEntity* EHandle::Get(const EntityList& entityList) const
{
	return entityList.GetEntityByHandle(*this);
}

void EHandle::Set(BaseEntity* entity)
{
	*this = entity ? entity->GetEntHandle() : EHandle{};
}
